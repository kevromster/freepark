#include "TFApp.h"
#include "TFException.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <iomanip>
#include <iostream>

TFApp::TFApp(const std::string & _strGraphFile) :
	m_strGraphFile(_strGraphFile)
{
}

void TFApp::detect(const std::string & _strImageFile, const std::string & _strOutFile) {
	const double dThreshold = 0.5;
	std::cout << "Detecting objects for '" << _strImageFile << "' with thresold " << dThreshold << "..." << std::endl;
	std::unique_ptr<TFDetectedObjects> objects = _getDetector().detectObjects(_strImageFile, dThreshold, std::vector<long>());

	std::cout
		<< "Found objects count: " << objects->getDetectedObjects().size() << std::endl
		<< "Drawing output bboxes..." << std::endl;
	_drawBBoxes(_strImageFile, _strOutFile, *objects);

	std::cout << "Detection finished." << std::endl;
}

ITFDetector & TFApp::_getDetector() {
	if (!m_pDetector)
		m_pDetector = ITFDetector::createDetector(m_strGraphFile);
	return *m_pDetector;
}

void TFApp::_drawBBoxes(
	const std::string & _strImageFile,
	const std::string & _strOutFile,
	const TFDetectedObjects & _objects
) {
	cv::Mat image = cv::imread(_strImageFile);
	if (!image.data)
		throw TFImageReadException(_strImageFile);

	const cv::Scalar redColor = CV_RGB(255, 0, 0);

	for (const TFDetectedObject & obj : _objects.getDetectedObjects()) {
		const cv::Rect & bbox = obj.getBBox();
		cv::rectangle(image, bbox, redColor);

		std::stringbuf sbuf;
		std::ostream os(&sbuf);
		os << std::fixed << std::setprecision(2) << obj.getScore()*100 << "% id" << obj.getId()
		   << " classId" << obj.getClassId();

		cv::putText(
			image,
			sbuf.str().c_str(),
			cv::Point(bbox.x+2, bbox.y+12),
			CV_FONT_HERSHEY_PLAIN,
			1.,
			redColor
		);

		// draw hull
		if (!obj.getConvexHull().empty()) {
			std::vector<std::vector<cv::Point>> hulls;
			hulls.push_back(obj.getConvexHull());
			cv::drawContours(image, hulls, 0, CV_RGB(0, 255, 0));
		}
	}

	cv::imwrite(_strOutFile, image);
}
