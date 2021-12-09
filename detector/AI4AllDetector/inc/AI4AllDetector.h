#ifndef AI4AllDetector_H
#define AI4AllDetector_H

#include <ITFDetector.h>

#include <opencv2/core/mat.hpp>

#include <list>
#include <map>
#include <string>

class SubmitCameraItem;

class AI4AllDetector
{
public:
	AI4AllDetector(
		const std::string & _carsGraphFile,
		const std::string &_generalPurposeGraphFile,
		const std::string & _tfLabelFile
	);

	void run(const SubmitCameraItem & _item);

private:
	cv::Mat _obtainScreenshotDirectlyFromNetwork(const std::string & _ip) const;
	cv::Mat _getScreenshotFromVM(const std::string & _fullScreenshotFileName) const;
	cv::Mat _cropScreenshot(const cv::Mat & _screenshot, const SubmitCameraItem & _item) const;

	bool _submitRequestForVM(const std::string & _urlFile, const std::string & _url) const;
	std::string _getVMScreenshotRequestDir(uint64_t _requestId) const;

	void _parseTensorFlowLabelsFile(const std::string & _strLabelsFile);
	void _rotateVmRootDirs() const;

	// Proccesses delayed items and searches for the given item in the delayed list. Returns true if found.
	bool _processDelayedItems(const SubmitCameraItem * _pItemToFind);
	void _submitDelayedDetectionItem(const SubmitCameraItem & _item);

	void _runFor(const SubmitCameraItem & _item, const cv::Mat & _screenshot) const;
	std::unique_ptr<TFDetectedObjects> _runDetection(const cv::Mat & _image, long _objectLabelId, double _detectionThreshold) const;

	void _sendMessageToTelegram(const SubmitCameraItem & _item, const cv::Mat & _inputScreenshot, const TFDetectedObjects & _detectedObjects) const;
	void _drawBBoxes(cv::Mat & _image, const TFDetectedObjects & _detectedObjects) const;

	// Returns labelId or -1 if the label wasn't found.
	long _getLabelId(const std::string & _whatToDetect) const;

private:
	const std::unique_ptr<ITFDetector> m_pCarsDetector;
	const std::unique_ptr<ITFDetector> m_pGeneralPurposeDetector;

	uint64_t m_uVMScreenshotRequestId;
	std::map<std::string, long> m_labelIds;

	// list of pairs <requestId, SubmitCameraItem>
	std::list<std::pair<uint64_t, SubmitCameraItem>> m_delayedItems;
};

#endif // AI4AllDetector_H
