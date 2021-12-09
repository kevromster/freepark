#ifndef ParkingRectangle_H
#define ParkingRectangle_H

#include <string>

#include <opencv2/core.hpp>

class ParkingRectangle
{
public:
	ParkingRectangle(const std::string & _strName, const cv::Rect & _rect, double _dRotationDegree) :
		m_strParkingName(_strName), m_rectangle(_rect), m_dRotationDegree(_dRotationDegree)
	{}

	const std::string & getName() const {return m_strParkingName;}
	const cv::Rect & getRectangle() const {return m_rectangle;}
	double getRotationDegree() const {return m_dRotationDegree;}

private:
	const std::string m_strParkingName;
	const cv::Rect m_rectangle;
	const double m_dRotationDegree;
};

#endif // ParkingRectangle_H
