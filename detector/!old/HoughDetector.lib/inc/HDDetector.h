#ifndef HDDetector_H
#define HDDetector_H

#include "IHDDetector.h"
#include "HoughForest.h"
#include "DetectOptions.h"

class HDDetectedObject;

class HDDetector : public IHDDetector
{
public:
	HDDetector(const std::string & _strConfigFile);

	std::unique_ptr<HDDetectedObjects> detectObjects(const std::string & _strInputImageFile);
	std::unique_ptr<HDDetectedObjects> detectObjects(const cv::Mat & _inputImage);

private:
	void _loadTrainedForest();
	std::unique_ptr<HDDetectedObjects> _runDetection(const cv::Mat & _inputImage);

	std::vector<cv::Mat> _detectForAllScales(const cv::Mat & _inputImage) const;
	void _detect(const cv::Mat & _inputImage, cv::Mat & _detectedImage) const;
	void _vote(const std::vector<const HoughLeaf*> & _leafs, int _cx, int _cy, cv::Mat & _detectedImage) const;

	std::vector<HDDetectedObject> _findBBoxes(const cv::Mat & _origImage, const std::vector<cv::Mat> & _detectedHoughImages) const;

private:
	DetectOptions m_options;
	HoughForest m_forest;
	bool m_bTrainedForestLoaded;
};

#endif // HDDetector_H
