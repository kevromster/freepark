#ifndef TrainImageInfo_H
#define TrainImageInfo_H

#include <string>

#include <opencv2/core.hpp>

struct TrainImageInfo
{
	std::string m_strFileName;
	cv::Point m_center;
	cv::Rect m_bbox;
};

#endif // TrainImageInfo_H
