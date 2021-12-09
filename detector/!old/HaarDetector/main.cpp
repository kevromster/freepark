//
// Reads input image and finds cars with Haar cascade classifier.
//

#include <iostream>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>

static
int usage() {
	std::cout << "usage: HaarDetector <input_image>" << std::endl << "  cascade file 'cars3.xml' must present near the binary" << std::endl;
	return 1;
}

int main(int argc, const char** argv) {
	if (argc != 2)
		return usage();

	const std::string strCascadeFileName = "cars3.xml";  // Not acceptable for our examples!
	const std::string strImageFileName = argv[1];
	assert(!strImageFileName.empty());

	cv::Mat image = cv::imread(strImageFileName, cv::IMREAD_COLOR);
	if (!image.data) {
		std::cerr << "Error loading image " << strImageFileName << std::endl;
		return usage();
	}

	std::cout << "Processing image: " << strImageFileName << std::endl;;

	cv::Mat grayImage;
	cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
	cv::equalizeHist(grayImage, grayImage);

	cv::CascadeClassifier cascade;
	if (!cascade.load(strCascadeFileName)) {
		std::cerr << "Error loading cascade file " << strCascadeFileName << std::endl;
		return usage();
	}

	std::vector<cv::Rect> cars;
	cascade.detectMultiScale(grayImage, cars, 1.1, 1);
	std::cout << "Found cars count: " << cars.size() << std::endl;

	for (size_t i = 0; i < cars.size(); ++i)
		cv::rectangle(image, cars[i], cv::Scalar(0, 0, 255));

/*
	CvSize img_size = cvGetSize(img);
    CvSeq *object = cvHaarDetectObjects(
      img,
      cascade,
      storage,
      1.1, //1.1,//1.5, //-------------------SCALE FACTOR
      1, //2        //------------------MIN NEIGHBOURS
      0, //CV_HAAR_DO_CANNY_PRUNING
      cvSize(0,0),//cvSize( 30,30), // ------MINSIZE
      img_size //cvSize(70,70)//cvSize(640,480)  //---------MAXSIZE
      );
*/

	cv::namedWindow("the image");
	cv::imshow("the image", image);

	cv::waitKey(0);
	return 0;
}
