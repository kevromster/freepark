//
// Initial app. Reads input image and finds borders using Canny's algorithm.
//

#include <iostream>

#include <opencv2/highgui/highgui.hpp>

#include "CannyApplier.h"
#include "CannyWindow.h"

static
int usage() {
	std::cout << "usage: freepark <input_image>" << std::endl;
	return 1;
}

int main(int argc, const char** argv) {
	if (argc != 2)
		return usage();

	const std::string strImageFileName = argv[1];
	assert(!strImageFileName.empty());

	cv::Mat image = cv::imread(strImageFileName, cv::IMREAD_COLOR);
	if (!image.data) {
		std::cerr << "Error loading image " << strImageFileName << std::endl;
		return usage();
	}

	std::cout << "Processing image: " << strImageFileName << std::endl;;

	CannyApplier cannyApplier(image, 19, 47);
	cannyApplier.apply();

	ImageWindow originalImageWindow(image, "original", cv::WINDOW_AUTOSIZE);
	ImageWindow preprocessedImageWindow(cannyApplier.getPreprocessedImage(), "before Canny's process", cv::WINDOW_AUTOSIZE);
	CannyWindow resultImageWindow(cannyApplier, "result: Canny's work", cv::WINDOW_AUTOSIZE);

	originalImageWindow.show();
	preprocessedImageWindow.show();
	resultImageWindow.show();

	cv::waitKey(0);
	return 0;
}
