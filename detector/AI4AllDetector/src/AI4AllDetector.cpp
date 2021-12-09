#include "AI4AllDetector.h"
#include "AI4AllDetectorLog.h"
#include "AI4AllDetectorException.h"
#include "ServerCommunicator.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>
#include <opencv2/videoio/videoio.hpp>

#include <tgbot/tgbot.h>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <thread>

namespace {

const std::string TGBOT_TOKEN = "615963778:AAEJGRsv675frXPhVdCgIHOhuDX-h8vXEUQ";

// root directory to communicate with VM
const std::string VM_ROOT_DIR = "/mnt/bigstore/xproject/ai4all/ScreenshotRequests";
const std::string VM_URL_FILE_NAME = "input_url";
const std::string VM_SCREENSHOT_FILE_NAME = "screenshot.png";

// maximum number of seconds to wait trying to open the VM comunication files
const long VM_URL_FILE_WAIT_LIMIT_SECS = 2;
const long VM_SCREENSHOT_FILE_WAIT_LIMIT_SECS = 60;

// number of milliseconds to sleep before the next try to open VM communication files
const long VM_URL_FILE_SLEEP_INTERVAL_MSECS = 100;
const long VM_SCREENSHOT_FILE_SLEEP_INTERVAL_MSECS = 1000;

// TODO: unify common methods with CarDetector's CameraOptions.cpp
std::string & ltrim(std::string & _s) {
	_s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return _s;
}

std::string & rtrim(std::string & _s) {
	_s.erase(std::find_if(_s.rbegin(), _s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _s.end());
	return _s;
}

std::string & trim(std::string & _s) {
	return ltrim(rtrim(_s));
}

std::vector<std::string> split(const std::string & _s, char _chDelimiter) {
	std::vector<std::string> elements;
	std::stringstream ss(_s);
	std::string item;
	while (std::getline(ss, item, _chDelimiter))
		if (!item.empty())
			elements.push_back(item);
	return elements;
}

bool startsFrom(const std::string & _strCheck, const std::string & _strPrefix) {
	return
		_strCheck.size() >= _strPrefix.size() &&
		_strCheck.substr(0, _strPrefix.size()) == _strPrefix;
}

} // anonymous namespace

AI4AllDetector::AI4AllDetector(
	const std::string & _carsGraphFile,
	const std::string &_generalPurposeGraphFile,
	const std::string & _tfLabelFile
) :
	m_pCarsDetector(ITFDetector::createDetector(_carsGraphFile)),
	m_pGeneralPurposeDetector(ITFDetector::createDetector(_generalPurposeGraphFile)),
	m_uVMScreenshotRequestId(0)
{
	_parseTensorFlowLabelsFile(_tfLabelFile);
	_rotateVmRootDirs();
}

void AI4AllDetector::run(const SubmitCameraItem & _item) {
	const cv::Mat screenshot = _obtainScreenshotDirectlyFromNetwork(_item.getUrl());

	// if couldn't obtain direct screenshot, process item as delayed
	bool processAsDelayed = screenshot.data == nullptr && startsFrom(_item.getUrl(), "http");

	if (!screenshot.data && !processAsDelayed) {
		LOG(ERROR) << "Error getting screenshot from camera by URL '" << _item.getUrl() << "'";
		return;
	}

	if (!processAsDelayed) {
		_runFor(_item, screenshot);
		_processDelayedItems(nullptr);
		return;
	}

	bool foundInDelayed = _processDelayedItems(&_item);
	if (!foundInDelayed) {
		_submitDelayedDetectionItem(_item);
	}
}

void AI4AllDetector::_runFor(const SubmitCameraItem & _item, const cv::Mat & _screenshot) const {
	if (!_screenshot.data) {
		return;
	}

	HDLOG << "Got screenshot from camera '" << _item.getCameraName() << "'";
	const cv::Mat input = _cropScreenshot(_screenshot, _item);

	cv::imwrite("input_screenshot.png", _screenshot);
	cv::imwrite("input_detector.png", input);

	const long labelId = _getLabelId(_item.getWhatToDetect());
	if (labelId == -1) {
		LOG(ERROR) << "The type of object to detect is unsupported: '" << _item.getWhatToDetect() << "'";
		return;
	}

	HDLOG << "Running TensorFlow detection for '" << _item.getWhatToDetect() << "' with threshold " << _item.getDetectionThreshold() << "%...";
	std::unique_ptr<TFDetectedObjects> detectedObjects = _runDetection(input, labelId, _item.getDetectionThreshold() / 100.);
	HDLOG << "Found objects count: " << detectedObjects->getDetectedObjects().size() << " of type '" << _item.getWhatToDetect() << "'";

	const bool objectPresents = detectedObjects->getDetectedObjects().size() > 0;
	const bool presentedLastTime = _item.wasObjectPresentedLastTime();
	const bool objectAppeared = objectPresents && !presentedLastTime;
	const bool objectDisappeared = !objectPresents && presentedLastTime;

	const NotificationType notificationType = _item.getNotificationType();

	const bool needSendNotification =
		(notificationType == NotificationType::PRESENCE && objectPresents) ||
		(notificationType == NotificationType::ABSENCE && !objectPresents) ||
		(notificationType == NotificationType::APPEARANCE && (objectAppeared || objectDisappeared));

	if (needSendNotification) {
		_sendMessageToTelegram(_item, input, *detectedObjects);
	}

	if (objectPresents != presentedLastTime) {
		ServerCommunicator::setSubmitCameraItemObjectFound(_item.getId(), objectPresents);
	}
}

cv::Mat AI4AllDetector::_cropScreenshot(const cv::Mat & _screenshot, const SubmitCameraItem & _item) const {
	if (_item.getEdgeLeft() > 0 || _item.getEdgeTop() > 0 || _item.getEdgeRight() < 100 || _item.getEdgeBottom() < 100) {
		int imageWidth = _screenshot.cols;
		int imageHeight = _screenshot.rows;

		int cropLeft = static_cast<int>(std::lround((_item.getEdgeLeft() * imageWidth) / 100.));
		int cropTop = static_cast<int>(std::lround((_item.getEdgeTop() * imageHeight) / 100.));
		int cropRight = static_cast<int>(std::lround((_item.getEdgeRight() * imageWidth) / 100.));
		int cropBottom = static_cast<int>(std::lround((_item.getEdgeBottom() * imageHeight) / 100.));

		if (cropLeft >= 0 && cropLeft < imageWidth && cropRight > cropLeft && cropRight <= imageWidth &&
			cropTop >= 0 && cropTop < imageHeight && cropBottom > cropTop && cropBottom <= imageHeight) {

			cv::Rect roi(cropLeft, cropTop, cropRight - cropLeft, cropBottom - cropTop);
			return _screenshot(roi);
		}

		LOG(ERROR) << "Error cropping ROI on the image {width,height} {" << imageWidth << "," << imageHeight
				   << "} with the given edge parameters {left,top,right,bottom} {"
				   << _item.getEdgeLeft() << "," << _item.getEdgeTop() << "," << _item.getEdgeRight() << "," << _item.getEdgeBottom() << "}";
	}

	return _screenshot;
}

void AI4AllDetector::_sendMessageToTelegram(const SubmitCameraItem & _item, const cv::Mat & _inputScreenshot, const TFDetectedObjects & _detectedObjects) const {
	cv::Mat detected;
	_inputScreenshot.copyTo(detected);
	_drawBBoxes(detected, _detectedObjects);
	cv::imwrite("detected.png", detected);

	HDLOG << "Send message to Telegram chat...";
	TgBot::Bot bot(TGBOT_TOKEN);
	const std::string name = _item.getCameraName().empty() ? "<noname>" : _item.getCameraName();
	const std::string message =
		"Camera '" + name + "': " + std::to_string(_detectedObjects.getDetectedObjects().size()) +
		" " + _item.getWhatToDetect() + "(s) found";

	bot.getApi().sendPhoto(_item.getTgChatId(), TgBot::InputFile::fromFile("detected.png", "image/png"), message);
	HDLOG << "Sent message+photo to Telegram chat " << _item.getTgChatId() << ": '" << message << "'";
}

void AI4AllDetector::_rotateVmRootDirs() const {
	const std::string rootPrevPrev = VM_ROOT_DIR + "_prev_prev";
	const std::string rootPrev = VM_ROOT_DIR + "_prev";

	if (boost::filesystem::exists(rootPrevPrev)) {
		boost::filesystem::remove_all(rootPrevPrev);
	}
	if (boost::filesystem::exists(rootPrev)) {
		boost::filesystem::rename(rootPrev, rootPrevPrev);
	}
	if (boost::filesystem::exists(VM_ROOT_DIR)) {
		boost::filesystem::rename(VM_ROOT_DIR, rootPrev);
	}
	boost::filesystem::create_directory(VM_ROOT_DIR);
}

cv::Mat AI4AllDetector::_obtainScreenshotDirectlyFromNetwork(const std::string & _url) const {
	cv::VideoCapture capture(_url);
	if (!capture.isOpened()) {
		return cv::Mat();
	}

	cv::Mat cameraImage;
	if (!capture.read(cameraImage)) {
		return cv::Mat();
	}

	return cameraImage;
}

void AI4AllDetector::_submitDelayedDetectionItem(const SubmitCameraItem & _item) {
	const uint64_t requestId = m_uVMScreenshotRequestId++;
	const std::string requestDir = _getVMScreenshotRequestDir(requestId);

	if (!boost::filesystem::exists(requestDir)) {
		if (!boost::filesystem::create_directory(requestDir)) {
			throw ErrorCreatingScreenshotsDirException(requestDir);
		}
	}

	_submitRequestForVM(requestDir + "/" + VM_URL_FILE_NAME, _item.getUrl());
	m_delayedItems.emplace_back(requestId, _item);
}

std::string AI4AllDetector::_getVMScreenshotRequestDir(uint64_t _requestId) const {
	return VM_ROOT_DIR + "/id" + std::to_string(_requestId);
}

bool AI4AllDetector::_processDelayedItems(const SubmitCameraItem * _pItemToFind) {
	bool isFound = false;
	std::list<std::pair<uint64_t, SubmitCameraItem>>::iterator it = m_delayedItems.begin();

	while (it != m_delayedItems.end()) {
		const uint64_t requestId = it->first;
		const SubmitCameraItem & item = it->second;

		if (!isFound && _pItemToFind && *_pItemToFind == item) {
			isFound = true;
		}

		const std::string requestDir = _getVMScreenshotRequestDir(requestId);
		const cv::Mat screenshot = _getScreenshotFromVM(requestDir + "/" + VM_SCREENSHOT_FILE_NAME);

		if (screenshot.data) {
			_runFor(item, screenshot);

			// delete processed delayed item and its request dir
			m_delayedItems.erase(it++);
			if (!boost::filesystem::remove_all(requestDir)) {
				LOG(WARNING) << "Couldn't remove unneeded anymore VM requestDir " << requestDir;
			}
		}
		else {
			// still not ready to process, increment to the next one
			++it;
		}
	}

	return isFound;
}

bool AI4AllDetector::_submitRequestForVM(const std::string & _urlFile, const std::string & _url) const {
	HDLOG << "Writing url file for VM...";
	std::ofstream vmUrlFile;
	const auto startTryingTime = std::chrono::high_resolution_clock::now();

	for (;;) {
		vmUrlFile.open(_urlFile);

		if (vmUrlFile.is_open()) {
			vmUrlFile << _url;
			return true;
		}

		const auto currentTime = std::chrono::high_resolution_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(
				currentTime - startTryingTime).count() >= VM_URL_FILE_WAIT_LIMIT_SECS
		) {
			LOG(ERROR) << "TOO LONG to wait trying to open url file with VM: '"
					   << _urlFile << "', break";
			return false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(VM_URL_FILE_SLEEP_INTERVAL_MSECS));
	}
}

cv::Mat AI4AllDetector::_getScreenshotFromVM(const std::string & _fullScreenshotFileName) const {
	if (boost::filesystem::exists(_fullScreenshotFileName)) {
		// open for writing to get exclusive lock and check thereby that file is ready to be read
		std::ofstream vmScreenshotFile(_fullScreenshotFileName, std::ios::out | std::ios::app | std::ios::binary);
		if (vmScreenshotFile.is_open()) {
			vmScreenshotFile.close();
			return cv::imread(_fullScreenshotFileName);
		}
	}

	return cv::Mat();
}

std::unique_ptr<TFDetectedObjects> AI4AllDetector::_runDetection(const cv::Mat & _image, long _objectLabelId, double _detectionThreshold) const {
	std::vector<long> classIds;
	classIds.reserve(1);
	classIds.push_back(_objectLabelId);

	// TODO: avoid hardcode for cars here
	return (_objectLabelId == 3 ? m_pCarsDetector : m_pGeneralPurposeDetector)->detectObjects(_image, _detectionThreshold, classIds);
}

long AI4AllDetector::_getLabelId(const std::string & _whatToDetect) const {
	std::map<std::string, long>::const_iterator it = m_labelIds.find(_whatToDetect);
	return it != m_labelIds.end() ? it->second : -1;
}

void AI4AllDetector::_drawBBoxes(cv::Mat & _image, const TFDetectedObjects & _detectedObjects) const {
	for (const TFDetectedObject & obj : _detectedObjects.getDetectedObjects()) {
		const cv::Rect & bbox = obj.getBBox();
		cv::rectangle(_image, bbox, CV_RGB(255, 0, 0), 3);

		/*std::stringbuf sbuf;
		std::ostream os(&sbuf);
		os << std::fixed << std::setprecision(2) << obj.getScore()*100 << "%";

		cv::putText(
			_image,
			sbuf.str().c_str(),
			cv::Point(bbox.x, bbox.y-10),
			CV_FONT_HERSHEY_PLAIN,
			2.,
			CV_RGB(255, 0, 0),
			2
		);*/

		// draw hull
		if (!obj.getConvexHull().empty()) {
			std::vector<std::vector<cv::Point>> hulls;
			hulls.push_back(obj.getConvexHull());
			cv::drawContours(_image, hulls, 0, CV_RGB(0, 255, 0));
		}
	}
}

void AI4AllDetector::_parseTensorFlowLabelsFile(const std::string & _strLabelsFile) {
	//
	// !! Copy-pasted (with slight changes) from CarDetector's CameraOptions.cpp OptionsLoader::_parseTensorFlowLabelsFile()
	// !! TODO: extract to one common method
	//

	// parsing file with sections like
	//   item {
	//     name: "/m/01g317"
	//     id: 1
	//     display_name: "person"
	//   }

	m_labelIds.clear();

	if (_strLabelsFile.empty())
		throw BadTFLabelsFileException("bad tensorflow labels file name");

	std::ifstream in(_strLabelsFile);

	if (!in.is_open())
		throw BadTFLabelsFileException("Tensorflow labels file not found: " + _strLabelsFile);

	std::string line;
	std::vector<std::string> vSplittedLine;

	std::string strParsedLabel;
	int nParsedClassId = -1;
	bool bIsIdParsed = false;
	bool bDisplayNameFound = false;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		std::getline(in, line);
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		if (line == "}") {
			if (!bIsIdParsed || strParsedLabel.empty())
				throw BadTFLabelsFileException("Error parsing tensorflow labels file " + _strLabelsFile);

			HDLOG << "Setting known object class id " << nParsedClassId << " to \"" << strParsedLabel << "\"...";
			m_labelIds[strParsedLabel] = nParsedClassId;

			nParsedClassId = -1;
			strParsedLabel = "";
			bIsIdParsed = false;
			bDisplayNameFound = false;

			continue;
		}

		vSplittedLine = split(line, ' ');

		if (vSplittedLine.size() < 2) {
			throw BadTFLabelsFileException(
				std::string("Wrong option in tensorflow labels file at line ") + std::to_string(uLine));
		}

		const std::string & strFirst = trim(vSplittedLine[0]);
		const std::string & strSecond = trim(vSplittedLine[1]);

		if (strFirst == "item") {
			if (strSecond != "{" || vSplittedLine.size() != 2) {
				throw BadTFLabelsFileException(
					std::string("Wrong option in tensorflow labels file at line ") + std::to_string(uLine));
			}
			// new section starts
			continue;
		}

		vSplittedLine = split(line, ':');

		if (vSplittedLine.size() != 2) {
			throw BadTFLabelsFileException(
				std::string("Wrong option in tensorflow labels file at line ") + std::to_string(uLine));
		}

		const std::string & strOption = trim(vSplittedLine[0]);
		const std::string & strValue = trim(vSplittedLine[1]);

		if (strOption == "id") {
			nParsedClassId = std::stoi(strValue);
			bIsIdParsed = true;
		} else if (strOption == "display_name" || (strOption == "name" && !bDisplayNameFound)) {
			// "display_name" is always preferred overt just "name"
			strParsedLabel = strValue;
			strParsedLabel.erase(std::remove(strParsedLabel.begin(), strParsedLabel.end(), '"'), strParsedLabel.end());
			bDisplayNameFound = strOption == "display_name";
		}
	}
}
