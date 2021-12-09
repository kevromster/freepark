#include "TFDetector.h"
#include "TFException.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <fstream>

namespace {

void deleteTensors(std::vector<TF_Tensor*> * _pTensors) {
	if (_pTensors) {
		for (TF_Tensor * pTensor : *_pTensors)
			TF_DeleteTensor(pTensor);
	}
}

void deleteSession(TF_Session * pSession) {
	AutoDeleteWrapper<TF_Status> closeSessionStatus(TF_NewStatus(), &TF_DeleteStatus);
	TF_CloseSession(pSession, closeSessionStatus.get());

	AutoDeleteWrapper<TF_Status> deleteSessionStatus(TF_NewStatus(), &TF_DeleteStatus);
	TF_DeleteSession(pSession, deleteSessionStatus.get());
}

void deallocatorStub(void*, size_t, void*) {
	// do nothing
}

} // anonymous namespace

std::unique_ptr<ITFDetector> ITFDetector::createDetector(const std::string & _strGraphFile) {
	if (_strGraphFile.empty())
		throw TFLoadGraphException("No input Graph file name given");

	return std::make_unique<TFDetector>(_strGraphFile);
}

TFDetector::TFDetector(const std::string & _strGraphFile) :
	m_strGraphFile(_strGraphFile)
{
}

std::unique_ptr<TFDetectedObjects> TFDetector::detectObjects(
	const std::string & _strInputImageFile,
	double _dDetectionThreshold,
	const std::vector<long> & _classIdsToDetect
) {
	const cv::Mat inputImage = cv::imread(_strInputImageFile, CV_LOAD_IMAGE_COLOR);
	if (!inputImage.data)
		throw TFImageReadException(_strInputImageFile);

	return detectObjects(inputImage, _dDetectionThreshold, _classIdsToDetect);
}

std::unique_ptr<TFDetectedObjects> TFDetector::detectObjects(
	const cv::Mat & _inputImage,
	double _dDetectionThreshold,
	const std::vector<long> & _classIdsToDetect
) {
	_loadGraph();

	cv::Mat clonedImage;
	const cv::Mat * pInputImage = &_inputImage;

	if (!_inputImage.isContinuous()) {
		// if image data is not continuous, clone the image to make it continuous
		clonedImage = _inputImage.clone();
		assert(clonedImage.isContinuous());
		pInputImage = &clonedImage;
	}

	return _runDetection(*pInputImage, _dDetectionThreshold, _classIdsToDetect);
}

void TFDetector::_loadGraph() {
	if (m_graph.get()) {
		// already loaded
		return;
	}

	std::ifstream input(m_strGraphFile, std::ios::binary);
	if (!input.is_open()) {
		throw TFLoadGraphException("Error reading Graph file \"" + m_strGraphFile + "\"");
	}

	std::vector<char> buffer((std::istreambuf_iterator<char>(input)), (std::istreambuf_iterator<char>()));

	AutoDeleteWrapper<TF_Buffer> graphDefBuffer(TF_NewBufferFromString(buffer.data(), buffer.size()), &TF_DeleteBuffer);
	AutoDeleteWrapper<TF_ImportGraphDefOptions> opts(TF_NewImportGraphDefOptions(), &TF_DeleteImportGraphDefOptions);
	AutoDeleteWrapper<TF_Status> importStatus(TF_NewStatus(), &TF_DeleteStatus);
	AutoDeleteWrapper<TF_Graph> graph(TF_NewGraph(), &TF_DeleteGraph);

	TF_GraphImportGraphDef(graph.get(), graphDefBuffer.get(), opts.get(), importStatus.get());

	if (TF_GetCode(importStatus.get()) != TF_OK) {
		throw TFLoadGraphException(TF_Message(importStatus.get()));
	}

	m_graph = std::move(graph);
}

std::unique_ptr<TFDetectedObjects> TFDetector::_runDetection(
	const cv::Mat & _inputImage,
	double _dDetectionThreshold,
	const std::vector<long> & _classIdsToDetect
) {
	// expect only continuous image data here
	assert(_inputImage.isContinuous());

	// prepare input image tensor
	const std::vector<int64_t> dims = {1, _inputImage.rows, _inputImage.cols, _inputImage.channels()};

	assert(dims.size() <= INT_MAX);
	const size_t inputImageDataSize = static_cast<size_t>(_inputImage.rows * _inputImage.cols * _inputImage.channels());

	AutoDeleteWrapper<TF_Tensor> inputImageTensor(TF_NewTensor(
		TF_UINT8,
		dims.data(),
		static_cast<int>(dims.size()),
		_inputImage.data,
		inputImageDataSize,
		&deallocatorStub, // nothing to specifically remove
		nullptr
	), &TF_DeleteTensor);
	TF_Tensor * pInputImageTensor = inputImageTensor.get();

	// prepare tensors and target operations
	TF_Output imageTensor = { TF_GraphOperationByName(m_graph.get(), "image_tensor"), 0 };

	std::vector<TF_Output> outputs = {
		{ TF_GraphOperationByName(m_graph.get(), "num_detections"), 0 },
		{ TF_GraphOperationByName(m_graph.get(), "detection_boxes"), 0 },
		{ TF_GraphOperationByName(m_graph.get(), "detection_scores"), 0 },
		{ TF_GraphOperationByName(m_graph.get(), "detection_classes"), 0 }
	};

	// add 'detection_masks' operation if it is supported by the graph
	TF_Operation * pDetectionMasksOperation = TF_GraphOperationByName(m_graph.get(), "detection_masks");
	if (pDetectionMasksOperation)
		outputs.push_back({pDetectionMasksOperation, 0});

	std::vector<const TF_Operation*> targetOperations(outputs.size(), nullptr);
	for (size_t i = 0; i < outputs.size(); ++i)
		targetOperations[i] = outputs[i].oper;

	std::vector<TF_Tensor*> outputTensors(outputs.size(), nullptr);
	AutoDeleteWrapper<std::vector<TF_Tensor*>> outputTensorsDeleteWrapper(&outputTensors, &deleteTensors);

	// create session
	AutoDeleteWrapper<TF_SessionOptions> sessionOptions(TF_NewSessionOptions(), &TF_DeleteSessionOptions);
	AutoDeleteWrapper<TF_Status> createSessionStatus(TF_NewStatus(), &TF_DeleteStatus);

	AutoDeleteWrapper<TF_Session> session(
		TF_NewSession(m_graph.get(), sessionOptions.get(), createSessionStatus.get()),
		&deleteSession
	);

	if (TF_GetCode(createSessionStatus.get()) != TF_OK)
		throw TFDetectionException(std::string("Failed to create session: ") + TF_Message(createSessionStatus.get()));

	assert(outputs.size() <= INT_MAX);
	assert(targetOperations.size() <= INT_MAX);

	// run session
	AutoDeleteWrapper<TF_Status> runSessionStatus(TF_NewStatus(), &TF_DeleteStatus);
	TF_SessionRun(
		session.get(),
		nullptr,
		&imageTensor, &pInputImageTensor, 1, // array of one element
		outputs.data(), outputTensors.data(), static_cast<int>(outputs.size()),
		targetOperations.data(), static_cast<int>(targetOperations.size()),
		nullptr,
		runSessionStatus.get());

	if (TF_GetCode(runSessionStatus.get()) != TF_OK)
		throw TFDetectionException(std::string("Fail while session running: ") + TF_Message(runSessionStatus.get()));

	std::vector<TFDetectedObject> bboxes = _obtainDetectedObjects(
		outputTensors[1], // boxes
		outputTensors[2], // scores
		outputTensors[3], // classes
		pDetectionMasksOperation ? outputTensors[4] : nullptr, // masks if presents
		_inputImage.cols,
		_inputImage.rows,
		_dDetectionThreshold,
		_classIdsToDetect
	);

	return std::make_unique<TFDetectedObjects>(std::move(bboxes));
}

std::vector<TFDetectedObject> TFDetector::_obtainDetectedObjects(
	const TF_Tensor * _pBoxesTensor,
	const TF_Tensor * _pScoresTensor,
	const TF_Tensor * _pClassesTensor,
	const TF_Tensor * _pMasksTensor,  // can be null, not all tensorflow graphs support mask detection
	int _nImageWidth,
	int _nImageHeight,
	double _dDetectionThreshold,
	const std::vector<long> & _classIdsToDetect
) const {
	std::vector<TFDetectedObject> bboxes;

	assert(_pBoxesTensor && _pScoresTensor && _pClassesTensor);
	if (!_pBoxesTensor || !_pScoresTensor || !_pClassesTensor)
		return bboxes;

	TF_DataType boxesDataType = TF_TensorType(_pBoxesTensor);
	TF_DataType scoresDataType = TF_TensorType(_pScoresTensor);
	TF_DataType classesDataType = TF_TensorType(_pClassesTensor);

	assert(boxesDataType == TF_FLOAT && scoresDataType == TF_FLOAT && classesDataType == TF_FLOAT);
	if (boxesDataType != TF_FLOAT || scoresDataType != TF_FLOAT || classesDataType != TF_FLOAT)
		return bboxes;

	// Boxes tensor looks like:
	//   length in dim 0 = 1
	//   length in dim 1 = 100
	//   length in dim 2 = 4
	// i.e. it contains 100 vectors with [ymin, xmin, ymax, xmax] values.

	int boxesNumDims = TF_NumDims(_pBoxesTensor);
	size_t boxesNumElements = 1;

	for (int dimIdx = 0; dimIdx < boxesNumDims; ++dimIdx)
		boxesNumElements *= static_cast<uint64_t>(TF_Dim(_pBoxesTensor, dimIdx));

	assert(boxesNumElements == TF_TensorByteSize(_pBoxesTensor) / sizeof(float));

	const float * pBoxesFloatData = reinterpret_cast<const float*>(TF_TensorData(_pBoxesTensor));
	const float * pScoresFloatData = reinterpret_cast<const float*>(TF_TensorData(_pScoresTensor));
	const float * pClassesFloatData = reinterpret_cast<const float*>(TF_TensorData(_pClassesTensor));

	const float * pMasksFloatData = nullptr;
	size_t maskMatrixSize = 0;

	if (_pMasksTensor) {
		TF_DataType masksDataType = TF_TensorType(_pMasksTensor);

		if (masksDataType == TF_FLOAT) {
			pMasksFloatData = reinterpret_cast<const float*>(TF_TensorData(_pMasksTensor));

			// Masks tensor looks like:
			//   length in dim 0 = 1
			//   length in dim 1 = 100
			//   length in dim 2 = 33
			//   length in dim 3 = 33
			// i.e. it contains 100 matrices of size 33x33.
			// Each matrix value is a probability to find object
			// at the corresponding pixel inside a detected bounding box.

			// should be 33*33 = 1089
			maskMatrixSize = TF_TensorByteSize(_pMasksTensor) / TF_TensorByteSize(_pClassesTensor);
		}
	}

	for (size_t idx = 0, boxIdx = 0, maskIdx = 0;
		 boxIdx < boxesNumElements;
		 boxIdx += 4, maskIdx += maskMatrixSize, ++idx
	) {
		const long lClassId = std::lround(pClassesFloatData[idx]);

		if (!_classIdsToDetect.empty() &&
			std::find(_classIdsToDetect.begin(), _classIdsToDetect.end(), lClassId) == _classIdsToDetect.end()
		) {
			// found class id is not in the list of the ones the user interested in
			continue;
		}

		const double dScore = static_cast<double>( pScoresFloatData[idx]);

		if (dScore > _dDetectionThreshold) {
			const float ymin = pBoxesFloatData[boxIdx];
			const float xmin = pBoxesFloatData[boxIdx+1];
			const float ymax = pBoxesFloatData[boxIdx+2];
			const float xmax = pBoxesFloatData[boxIdx+3];

			// denormalize coordinates
			cv::Rect bbox(
				static_cast<int>(std::lround(xmin * _nImageWidth)),
				static_cast<int>(std::lround(ymin * _nImageHeight)),
				static_cast<int>(std::lround((xmax - xmin) * _nImageWidth)),
				static_cast<int>(std::lround((ymax - ymin) * _nImageHeight))
			);

			std::vector<cv::Point> convexHull =
				_calculateConvexHull(bbox, pMasksFloatData, maskIdx, maskMatrixSize, _dDetectionThreshold);

			bboxes.emplace_back(bboxes.size(), lClassId, bbox, dScore, std::move(convexHull));
		}
	}

	return bboxes;
}

std::vector<cv::Point> TFDetector::_calculateConvexHull(
	const cv::Rect & _bbox,
	const float * _pMasksFloatData,
	size_t _nGlobalMaskShift,
	size_t _nMaskMatrixSize,
	double _dDetectionThreshold
) const {
	if (!_pMasksFloatData)
		return std::vector<cv::Point>();

	// expect only square matrices, so there must be round square root
	const size_t nDimLen = static_cast<size_t>(std::lround(std::sqrt(_nMaskMatrixSize)));

	const double coefX = _bbox.width / static_cast<double>(nDimLen);
	const double coefY = _bbox.height / static_cast<double>(nDimLen);

	// divide threshold by 4 - selected empirically
	const double dMaskThreshold = _dDetectionThreshold / 4.;

	size_t bboxX = 0;
	size_t bboxY = 0;
	std::vector<cv::Point> maskPoints;

	// collect pixels on image with high probability to find the object
	for (size_t idx = 0; idx < _nMaskMatrixSize; ++idx) {
		const double dMaskValue = static_cast<double>(_pMasksFloatData[_nGlobalMaskShift + idx]);

		if (dMaskValue > dMaskThreshold) {
			// translate from bbox cooordinates to image ones
			maskPoints.emplace_back(_bbox.x + std::lround(bboxX * coefX), _bbox.y + std::lround(bboxY * coefY));
		}

		++bboxX;
		if (bboxX >= nDimLen) {
			bboxX = 0;
			++bboxY;
		}
	}

	// calculate convex hull from the collected points
	std::vector<cv::Point> convexHull;

	if (!maskPoints.empty())
		cv::convexHull(maskPoints, convexHull);

	return convexHull;
}
