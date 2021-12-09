#include "HoughLeaf.h"
#include "HDException.h"
#include "HDLibLog.h"

#include <opencv2/highgui.hpp>

#include <fstream>

HoughLeaf::HoughLeaf() :
	m_dForegroundProbability(0.)
{}

HoughLeaf::HoughLeaf(double _dForegroundProbability, size_t _sizeOfPatchCenters) :
	m_dForegroundProbability(_dForegroundProbability)
{
	m_vPatchCenters.reserve(_sizeOfPatchCenters);
}

void HoughLeaf::addPatchCenter(const cv::Point & _pt) {
	m_vPatchCenters.push_back(_pt);
}

void HoughLeaf::showPatchCenters() const {
	std::ostringstream oss;
	oss << "leaf: fg probability = " << m_dForegroundProbability;
	const std::string strLeafName = oss.str();
	LOG(INFO) << strLeafName << ", patch centers count = " << m_vPatchCenters.size();

	if (m_vPatchCenters.size() == 0)
		return;

	const int nWidth = 100;
	const int nHeight = 100;

	cv::Mat image(cv::Size(nWidth, nHeight), CV_8UC1, cv::Scalar(0));

	for (const cv::Point & pt : m_vPatchCenters) {
		int x = nWidth/2 + pt.x;
		int y = nHeight/2 + pt.y;

		if(x >= 0 && y >= 0 && x < nWidth && y < nHeight)
			image.at<uchar>(cv::Point(x, y)) = 255;
	}

	cv::imshow(strLeafName, image);
	cv::waitKey();
	cv::destroyWindow(strLeafName);
}

void HoughLeaf::save(std::ofstream & _stream) const {
	_stream << getForegroundProbability() << " " << getPatchCenters().size();

	for (const cv::Point & patchCenter : getPatchCenters())
		_stream << " " << patchCenter.x << " " << patchCenter.y;
}

void HoughLeaf::load(std::ifstream & _stream) {
	double dForegroundProbability = 0.;
	_stream >> dForegroundProbability;

	unsigned int uPatchCentersCount = 0;
	_stream >> uPatchCentersCount;

	std::vector<cv::Point> patchCenters;
	patchCenters.reserve(uPatchCentersCount);

	for (unsigned int uIdx = 0; uIdx < uPatchCentersCount; ++uIdx) {
		patchCenters.emplace_back();
		_stream >> patchCenters.back().x;
		_stream >> patchCenters.back().y;
	}

	m_dForegroundProbability = dForegroundProbability;
	m_vPatchCenters.swap(patchCenters);
}
