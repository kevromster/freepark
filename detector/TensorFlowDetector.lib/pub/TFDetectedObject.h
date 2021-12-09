#ifndef TFDetectedObject_H
#define TFDetectedObject_H

#include "TFExport.h"

#include <opencv2/core.hpp>

/**
 * Represents information about detected object on an image.
 */
class TF_API TFDetectedObject
{
public:
	TFDetectedObject(long _lId, long _lClassId, const cv::Rect & _bbox, double _dScore, std::vector<cv::Point> && _convexHull) :
		m_lId(_lId),
		m_lClassId(_lClassId),
		m_bbox(_bbox),
		m_dScore(_dScore),
		m_convexHull(_convexHull)
	{}

	long getId() const {return m_lId;}
	long getClassId() const {return m_lClassId;}
	double getScore() const {return m_dScore;}
	const cv::Rect & getBBox() const {return m_bbox;}
	const std::vector<cv::Point> & getConvexHull() const {return m_convexHull;}

private:
	long m_lId;        // unique object id through the detected ones
	long m_lClassId;   // class that object belongs to, all classes are defined in labels map file used to train the model
	cv::Rect m_bbox;   // detected object bounding box
	double m_dScore;   // probability from 0 to 1 of a correct detection

	// convex hull (more precise contour) of the detected object,
	// can be empty if tensorflow graph used to detect objects doesn't support detection masks
	std::vector<cv::Point> m_convexHull;
};

#endif // TFDetectedObject_H
