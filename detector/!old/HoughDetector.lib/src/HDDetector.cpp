#include "HDDetector.h"

#include "HDException.h"
#include "PatchStorage.h"
#include "HDLibLog.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace {

struct MaxLocation
{
	MaxLocation(unsigned int _uScaleIdx) :
		m_uScaleIdx(_uScaleIdx), m_dValue(0.)
	{}

	bool operator< (const MaxLocation & _other) const {
		return m_dValue < _other.m_dValue;
	}

	unsigned int m_uScaleIdx;
	cv::Point m_point;
	double m_dValue;
};

} // anonymous namespace

std::unique_ptr<IHDDetector> IHDDetector::createDetector(const std::string & _strConfigFile) {
	HDInitializeLogger();
	return std::make_unique<HDDetector>(_strConfigFile);
}

HDDetector::HDDetector(const std::string & _strConfigFile) :
	m_options(DetectOptions::createFromFile(_strConfigFile)),
	m_bTrainedForestLoaded(false)
{
}

std::unique_ptr<HDDetectedObjects> HDDetector::detectObjects(const std::string & _strInputImageFile) {
	const cv::Mat inputImage = cv::imread(_strInputImageFile);
	if (!inputImage.data)
		throw ImageReadException(_strInputImageFile);

	return detectObjects(inputImage);
}

std::unique_ptr<HDDetectedObjects> HDDetector::detectObjects(const cv::Mat & _inputImage) {
	_loadTrainedForest();
	return _runDetection(_inputImage);
}

void HDDetector::_loadTrainedForest() {
	if (m_bTrainedForestLoaded) {
		HDLOG << "Trained forest already loaded.";
		return;
	}
	HDLOG << "Loading forest...";
	m_forest.load(m_options.getTrainedForestPath());
	m_bTrainedForestLoaded = true;
	HDLOG << "Forest loaded.";
}

std::unique_ptr<HDDetectedObjects> HDDetector::_runDetection(const cv::Mat & _inputImage) {
	HDLOG << "Run images detection...";
	std::vector<cv::Mat> houghImages = _detectForAllScales(_inputImage);
	std::vector<HDDetectedObject> bboxes = _findBBoxes(_inputImage, houghImages);
	HDLOG << "Detection succeeded.";

	return std::make_unique<HDDetectedObjects>(std::move(bboxes), std::move(houghImages));
}

std::vector<HDDetectedObject> HDDetector::_findBBoxes(const cv::Mat & _origImage, const std::vector<cv::Mat> & _detectedHoughImages) const {
	HDLOG << "Looking for max locations and bounding boxes around detected objects...";
	std::vector<cv::Mat> houghImages;
	std::vector<HDDetectedObject> result;

	for (const cv::Mat & _image : _detectedHoughImages)
		houghImages.push_back(_image.clone());

	for (long lId = 0;; ++lId) {
		MaxLocation maxLoc(0);

		// find max location through all scaled hough images
		for(unsigned int uScaleIdx = 0; uScaleIdx < houghImages.size(); ++uScaleIdx) {

			MaxLocation curMaxLoc(uScaleIdx);
			cv::minMaxLoc(houghImages[uScaleIdx], NULL, &curMaxLoc.m_dValue, NULL, &curMaxLoc.m_point);

			if (maxLoc < curMaxLoc)
				maxLoc = curMaxLoc;
		}

		LOG(INFO) << "found max value: " << maxLoc.m_dValue << ", scale: " << m_options.getScales()[maxLoc.m_uScaleIdx];
		if (maxLoc.m_dValue < m_options.getDetectionThreshold())
			break;

		// draw bbox around found max location
		const cv::Size szScaled(
			int(m_options.getTrainObjectSize().width/m_options.getScales()[maxLoc.m_uScaleIdx]+0.5),
			int(m_options.getTrainObjectSize().height/m_options.getScales()[maxLoc.m_uScaleIdx]+0.5)
		);
		const cv::Rect bboxScaled(
			int(maxLoc.m_point.x/m_options.getScales()[maxLoc.m_uScaleIdx] - szScaled.width/2. + 0.5),
			int(maxLoc.m_point.y/m_options.getScales()[maxLoc.m_uScaleIdx] - szScaled.height/2. + 0.5),
			szScaled.width,
			szScaled.height
		);

		result.emplace_back(lId, bboxScaled, m_options.getScales()[maxLoc.m_uScaleIdx], maxLoc.m_dValue);

		// zeroize bbox on all hough scaled images and search max location again
		for(unsigned int uScaleIdx = 0; uScaleIdx < houghImages.size(); ++uScaleIdx) {
			const double dScaleFactor = double(houghImages[uScaleIdx].size().width) / double(houghImages[maxLoc.m_uScaleIdx].size().width);
			const cv::Size sz(int(m_options.getTrainObjectSize().width*dScaleFactor + 0.5), int(m_options.getTrainObjectSize().height*dScaleFactor + 0.5));
			const cv::Rect bbox(int(maxLoc.m_point.x*dScaleFactor - sz.width/2. + 0.5), int(maxLoc.m_point.y*dScaleFactor - sz.height/2. + 0.5), sz.width, sz.height);

			// TODO: maybe use mask instead of filling found maximus by black color
			cv::rectangle(houghImages[uScaleIdx], bbox, CV_RGB(0, 0, 0), CV_FILLED);
		}
	}

	return result;
}

std::vector<cv::Mat> HDDetector::_detectForAllScales(const cv::Mat & _inputImage) const {
	std::vector<cv::Mat> vDetectedHoughImages;
	vDetectedHoughImages.reserve(m_options.getScales().size());

	for (double dScale : m_options.getScales()) {
		cv::Mat imageForDetection(cv::Size(), CV_8UC3);
		cv::resize(_inputImage, imageForDetection, cv::Size(), dScale, dScale);

		cv::Mat houghImage(imageForDetection.size(), CV_32FC1, cv::Scalar(0));
		_detect(imageForDetection, houghImage);

		vDetectedHoughImages.emplace_back(houghImage.size(), CV_8UC1);
		houghImage.convertTo(vDetectedHoughImages.back(), vDetectedHoughImages.back().type(), m_options.getDetectedScaling());
	}

	return vDetectedHoughImages;
}

void HDDetector::_detect(const cv::Mat & _inputImage, cv::Mat & _detectedImage) const {
	const std::vector<std::unique_ptr<cv::Mat>> imageChannels = PatchStorage::extractChannels(_inputImage);

	const int xoffset = m_options.getPatchSize().width/2;
	const int yoffset = m_options.getPatchSize().height/2;

	int x, y, cx, cy; // x,y top left; cx,cy center of patch
	cy = yoffset;

	for(y = 0; y < _inputImage.rows - m_options.getPatchSize().height; ++y, ++cy) {
		cx = xoffset;
		for(x = 0; x < _inputImage.cols - m_options.getPatchSize().width; ++x, ++cx)
			_vote(m_forest.regression(x, y, imageChannels), cx, cy, _detectedImage);
	}

	// smooth result image
	cv::GaussianBlur(_detectedImage, _detectedImage, cv::Size(3, 3), 0, 0);
}

void HDDetector::_vote(const std::vector<const HoughLeaf*> & _leafs, int _cx, int _cy, cv::Mat & _detectedImage) const {
	for (const auto & leaf : _leafs) {
		// To speed up the voting, one can vote only for patches with a probability for foreground > 0.5
		// if((*itL)->pfg>0.5) {

		if (leaf->getPatchCenters().size() == 0)
			continue;

		// voting weight for leaf
		const double dWeight = leaf->getForegroundProbability() / double(leaf->getPatchCenters().size() * _leafs.size());
		const float fWeight = static_cast<float>(dWeight);

		// vote for all points stored in the leaf
		for (const cv::Point & patchCenter : leaf->getPatchCenters()) {
			const int xDetected = _cx - patchCenter.x;
			const int yDetected = _cy - patchCenter.y;

			if(yDetected >= 0 && yDetected < _detectedImage.rows && xDetected >= 0 && xDetected < _detectedImage.cols)
				_detectedImage.at<float>(cv::Point(xDetected, yDetected)) += fWeight;
		}
	}
}
