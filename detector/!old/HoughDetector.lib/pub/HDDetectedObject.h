#ifndef HDDetectedObject_H
#define HDDetectedObject_H

#include "HDExport.h"

#include <opencv2/core.hpp>

/**
 * Represents information about detected object on an image.
 */
class HD_API HDDetectedObject
{
public:
	HDDetectedObject(long _lId, const cv::Rect & _bbox, double _dScale, double _dValue) :
		m_lId(_lId),
		m_bbox(_bbox),
		m_dMaxLocationScale(_dScale),
		m_dMaxLocationValue(_dValue)
	{}

	long getId() const {return m_lId;}
	const cv::Rect & getBBox() const {return m_bbox;}

	double getScale() const {return m_dMaxLocationScale;}
	double getValue() const {return m_dMaxLocationValue;}

private:
	long m_lId;
	cv::Rect m_bbox;
	double m_dMaxLocationScale;
	double m_dMaxLocationValue;
};

#endif // HDDetectedObject_H
