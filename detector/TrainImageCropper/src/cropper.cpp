/*
 * Crops input screenshot for parking images on which to train next.
 */

#include "Parkings.h"

#include <iostream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

namespace {

struct MatchPathSeparator
{
	bool operator()(char _ch) const {
		return _ch == '\\' || _ch == '/';
	}
};

std::string getFileName(const std::string & _strPath) {
	return std::string(
		std::find_if(_strPath.rbegin(), _strPath.rend(), MatchPathSeparator()).base(),
		_strPath.end()
	);
}

cv::Mat rotateImage(const cv::Mat & _image, double _degree) {
	cv::Point2d center(_image.cols/2.0, _image.rows/2.0);
	cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, _degree, 1.0);

	cv::Rect bbox = cv::RotatedRect(center, _image.size(), static_cast<float>(_degree)).boundingRect();
	rotationMatrix.at<double>(0,2) += bbox.width/2.0 - center.x;
	rotationMatrix.at<double>(1,2) += bbox.height/2.0 - center.y;

	cv::Mat rotatedImage;
	cv::warpAffine(_image, rotatedImage, rotationMatrix, bbox.size());
	return rotatedImage;
}

int usage() {
	std::cout
		<< "Usage: TrainImageCropper <config-file> <image-file> <out-dir>" << std::endl
		<< "    The application reads parking rectangles from the configuration file," << std::endl
		<< "    crops parkings from the input image and stores them in <out-dir> with name" << std::endl
		<< "    <parking_name>_<image-filename>" << std::endl;
	return 1;
}

} // anonymous namespace

int main(int argc, char** argv) {
	try {
		if (argc != 4)
			return usage();

		const std::string strConfigFile = argv[1];
		const std::string strImagePath = argv[2];
		const std::string outDir = argv[3];

		const Parkings parkings(strConfigFile);
		const cv::Mat image = cv::imread(strImagePath);

		for (const ParkingRectangle & p : parkings.getParkings()) {
			cv::Mat cropped;
			if (std::abs(p.getRotationDegree()) > 1.e-6) {
				cv::Mat rotated = rotateImage(image, p.getRotationDegree());
				cropped = rotated(p.getRectangle());
			} else {
				cropped = image(p.getRectangle());
			}
			cv::imwrite(outDir + "/" + p.getName() + "_" + getFileName(strImagePath), cropped);
		}

		return 0;
	} catch (Parkings::ParseException & _ex) {
		std::cerr << "ParseException caught: " << _ex.message() << std::endl;
		return 1;
	} catch (const std::exception & _ex) {
		std::cerr << "std::exception caught: " << _ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << "Unknown exception caught! Exit from app.." << std::endl;
		return 1;
	}
}
