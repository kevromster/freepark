#include "CannyApplier.h"

#include <opencv2/imgproc/imgproc.hpp>

CannyApplier::CannyApplier(const cv::Mat & _initialImage, double _dLowThreshold, double _dHighThreshold) :
	m_dLowThreshold(_dLowThreshold), m_dHighThreshold(_dHighThreshold), m_initialImage(_initialImage)
{
	_prepareForCanny(m_initialImage, m_preprocessedImage);
}

void CannyApplier::apply() {
	cv::Mat cannyImage;
	cv::Canny(m_preprocessedImage, cannyImage, m_dLowThreshold, m_dHighThreshold);

	std::vector<std::vector<cv::Point>> contours;
	std::vector<std::vector<cv::Point>> approxedContours;

	/// Find contours
	cv::findContours(cannyImage, contours, cv::/*RETR_CCOMP*/RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	approxedContours.reserve(contours.size());
	m_resultImage = cv::Mat::zeros(cannyImage.size(), CV_8UC3);
	cv::RNG rng(12345);

	for(size_t i = 0; i < contours.size(); ++i) {
		std::vector<cv::Point> approxed;
		cv::approxPolyDP(contours[i], approxed, 1, true);
		approxedContours.push_back(approxed);

//		const double dArea = fabs(cv::contourArea(approxed));
//		const double dPerimeter = cv::arcLength(approxed, true);

		// car: length more or equal to double width, then:
		//if (dArea * 18. >= dPerimeter*dPerimeter)
			cv::drawContours(m_resultImage, contours, i, cv::Scalar(rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255)));
/*
		//if (dArea/dPerimeter <= 2.)
		if (dArea * 18. < dPerimeter*dPerimeter)
			continue;
*/
/*
		cv::Rect rect = cv::boundingRect(contours[i]);
		cv::Point pt1, pt2;
		pt1.x = rect.x;
		pt2.x = (rect.x + rect.width);
		pt1.y = rect.y;
		pt2.y = (rect.y + rect.height);
		double ratio= rect.width > rect.height ? rect.width/rect.height : rect.height/rect.width;

		//if ((2.0 < fabs(ratio) && fabs(ratio) < 8.0))
		if (fabs(ratio) > 2.)
			cv::rectangle(m_resultImage, pt1, pt2, cv::Scalar(0, 0, 255));
*/
	}
}

void CannyApplier::_prepareForCanny(const cv::Mat & _initialImage, cv::Mat & _preparedImage) const {
	cv::cvtColor(_initialImage, _preparedImage, cv::COLOR_BGR2GRAY);
	cv::GaussianBlur(_preparedImage, _preparedImage, cv::Size(3, 3), 0, 0);

	const cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	cv::erode(_preparedImage, _preparedImage, element);
	cv::dilate(_preparedImage, _preparedImage, element);
}
