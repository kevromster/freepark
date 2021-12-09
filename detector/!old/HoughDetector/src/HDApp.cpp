#include "HDApp.h"
#include "HDException.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

HDApp::HDApp(const std::string & _strConfigFile) :
	m_strConfigFile(_strConfigFile)
{
}

void HDApp::detect(const std::string & _strImageFile, const std::string & _strOutFile) {
	std::unique_ptr<HDDetectedObjects> objects = _getDetector().detectObjects(_strImageFile);
	_drawBBoxes(_strImageFile, _strOutFile, *objects);
	_saveHoughImages(_strOutFile, *objects);
}

void HDApp::train() {
	_getTrainer().train();
}

IHDDetector & HDApp::_getDetector() {
	if (!m_pDetector)
		m_pDetector = IHDDetector::createDetector(m_strConfigFile);
	return *m_pDetector;
}

IHDTrainer & HDApp::_getTrainer() {
	if (!m_pTrainer)
		m_pTrainer = IHDTrainer::createTrainer(m_strConfigFile);
	return *m_pTrainer;
}

void HDApp::_drawBBoxes(const std::string & _strImageFile, const std::string & _strOutFile, const HDDetectedObjects & _objects) {
	cv::Mat inputImage = cv::imread(_strImageFile, cv::IMREAD_COLOR);
	if (!inputImage.data)
		throw ImageReadException(_strImageFile);

	for (const HDDetectedObject & obj : _objects.getDetectedObjects()) {
		const cv::Rect & bbox = obj.getBBox();
		cv::rectangle(inputImage, bbox, CV_RGB(255, 0, 0));

		std::stringbuf sbuf;
		std::ostream os(&sbuf);
		os << obj.getValue() << " " << obj.getScale() << " id" << obj.getId() << " {" << bbox.x << "," << bbox.y << "," << bbox.width << "," << bbox.height << "}";
		cv::putText(inputImage, sbuf.str().c_str(), cv::Point(bbox.x+2, bbox.y+12), CV_FONT_HERSHEY_PLAIN, 1., CV_RGB(255, 0, 0));
	}

	cv::imwrite(_strOutFile, inputImage);
}

void HDApp::_saveHoughImages(const std::string &_strOutFile, const HDDetectedObjects & _objects) {
	unsigned int uIdx = 0;
	for (const cv::Mat & houghImage : _objects.getHoughImages()) {
		cv::imwrite("hough-scale" + std::to_string(uIdx++) + "-" + _strOutFile, houghImage);
	}
}
