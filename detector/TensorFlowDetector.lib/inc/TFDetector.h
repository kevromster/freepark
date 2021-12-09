#ifndef TFDetector_H
#define TFDetector_H

#include "ITFDetector.h"
#include "AutoDeleteWrapper.h"

#include <tensorflow/c/c_api.h>

class TFDetectedObject;

class TFDetector : public ITFDetector
{
public:
	TFDetector(const std::string & _strGraphFile);

	std::unique_ptr<TFDetectedObjects> detectObjects(
		const std::string & _strInputImageFile,
		double _dDetectionThreshold,
		const std::vector<long> & _classIdsToDetect
	);

	std::unique_ptr<TFDetectedObjects> detectObjects(
		const cv::Mat & _inputImage,
		double _dDetectionThreshold,
		const std::vector<long> & _classIdsToDetect
	);

private:
	void _loadGraph();

	std::unique_ptr<TFDetectedObjects> _runDetection(
		const cv::Mat & _inputImage,
		double _dDetectionThreshold,
		const std::vector<long> & _classIdsToDetect
	);

	std::vector<TFDetectedObject> _obtainDetectedObjects(
		const TF_Tensor * _pBoxesTensor,
		const TF_Tensor * _pScoresTensor,
		const TF_Tensor * _pClassesTensor,
		const TF_Tensor * _pMasksTensor,
		int _nImageWidth,
		int _nImageHeight,
		double _dDetectionThreshold,
		const std::vector<long> & _classIdsToDetect
	) const;

	std::vector<cv::Point> _calculateConvexHull(
		const cv::Rect & _bbox,
		const float * _pMasksFloatData,
		size_t _nGlobalMaskShift,
		size_t _nMaskMatrixSize,
		double _dDetectionThreshold
	) const;

private:
	const std::string m_strGraphFile;
	AutoDeleteWrapper<TF_Graph> m_graph;
};

#endif // TFDetector_H
