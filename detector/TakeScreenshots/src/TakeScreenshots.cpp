/*
 * Takes screenshots from the given camera URI.
 */

#include <iostream>
#include <chrono>
#include <thread>

#include <opencv2/highgui.hpp>

#include <yadisk/client.hpp>

namespace {

const std::string YADISK_OAUTH_TOKEN = "AQAAAAAmwdcpAAUjJG2iVbD2CEgUp6W_shaGz8w";

int usage() {
	std::cout
		<< "Usage: TakeScreenshots [--to-yandex-disk] <URI> <interval> <out-dir>" << std::endl
		<< "    The application connect to the given camera URI and takes a screenshot " << std::endl
		<< "    for every <interval> seconds putting it into <out-dir> directory (it must exist already)." << std::endl
		<< "    If --to-yandex-disk option specified, the saving is done onto Yandex.Disk under 'freepark.city' account." << std::endl;
	return 1;
}

cv::Mat captureImage(const std::string & _strCameraUri) {
	cv::VideoCapture capture(_strCameraUri);

	if (!capture.isOpened()) {
		std::cerr << "Error opening camera " << _strCameraUri << std::endl;
		return cv::Mat();
	}

	cv::Mat cameraImage;
	if (!capture.read(cameraImage)) {
		std::cerr << "Error reading frame from the camera " << _strCameraUri << std::endl;
		return cv::Mat();
	}

	return cameraImage;
}

std::string num2strWithTwoDigits(int num) {
	const std::string str = std::to_string(num);
	return num > 0 && num < 10 ? "0" + str : str;
}

std::string getScreenshotFileName() {
	std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::high_resolution_clock::now());
	std::tm* now = std::localtime(&t);

	const int year = now->tm_year + 1900;
	const int month = now->tm_mon + 1;
	const int day = now->tm_mday;
	const int hours = now->tm_hour;
	const int mins = now->tm_min;
	const int secs = now->tm_sec;

	// use the same format as in take_cam_screenshots.au3
	return
		"scr_" +
		std::to_string(year) +
		num2strWithTwoDigits(month) +
		num2strWithTwoDigits(day) +
		"_" +
		num2strWithTwoDigits(hours) +
		"-" +
		num2strWithTwoDigits(mins) +
		"-" +
		num2strWithTwoDigits(secs) +
		".png";
}

void saveScreenshotToDirectory(const std::string & _strOutDir, const cv::Mat & _screenshot) {
	const std::string strFullName = _strOutDir + "/" + getScreenshotFileName();
	std::cout << "Saving screenshot as '" << strFullName << "'..." << std::endl;

	std::vector<int> params;
	params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	params.push_back(9);

	if (cv::imwrite(strFullName, _screenshot, params))
		std::cout << "Saved successfully." << std::endl;
	else
		std::cout << "Some problems while saving!" << std::endl;
}

void saveScreenshotToYandexDisk(const std::string & _strOutDir, const cv::Mat & _screenshot) {
	url::path yandexDiskPath(_strOutDir);
	yandexDiskPath /= getScreenshotFileName();
	std::cout << "Saving screenshot to Yandex.Disk as '" << yandexDiskPath << "'..." << std::endl;

	std::vector<uchar> encodedImage;
	std::vector<int> params;
	params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	params.push_back(9);

	if (!cv::imencode(".png", _screenshot, encodedImage, params)) {
		std::cout << "Failed to encode the image into array of bytes" << std::endl;
		return;
	}

	yadisk::Client client(YADISK_OAUTH_TOKEN);
	json result = client.upload(yandexDiskPath, encodedImage.data(), encodedImage.size(), true);

	if (!result.empty())
		std::cout << "Saved successfully." << std::endl;
	else
		std::cout << "Some problems while saving!" << std::endl;
}

} // anonymous namespace

int main(int argc, char** argv) {
	try {
		if (argc != 4 && argc != 5)
			return usage();

		int nextArg = 1;
		const bool bSaveToYandexDisk = std::string(argv[nextArg]) == "--to-yandex-disk";
		if (bSaveToYandexDisk)
			nextArg++;

		const std::string strCameraUri = argv[nextArg++];
		const int nIntervalSeconds = std::stoi(argv[nextArg++]);
		const std::string strOutDir = argv[nextArg++];

		for (;;) {
			std::cout << "Capturing screenshot from " << strCameraUri << "..." << std::endl;
			cv::Mat screenshot = captureImage(strCameraUri);

			if (screenshot.data) {
				if (bSaveToYandexDisk)
					saveScreenshotToYandexDisk(strOutDir, screenshot);
				else
					saveScreenshotToDirectory(strOutDir, screenshot);
			}

			std::cout << "Sleeping for " << nIntervalSeconds << " seconds..." << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(nIntervalSeconds));
		}

		return 0;
	} catch (const std::exception & _ex) {
		std::cerr << "std::exception caught: " << _ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unknown exception caught! Exit from app.." << std::endl;
		return 1;
	}
}
