#ifndef ParkingPlace_H
#define ParkingPlace_H

#include "GeoCoordinatePoint.h"

#include <opencv2/core.hpp>

#include <iomanip>
#include <vector>

class Parking;

/**
 * Class to specify found free parking place.
 */
class ParkingPlace
{
public:
	enum PARK_COMPLEXITY {
		UNKNOWN = 0,
		NEWBIE,   // even newbie driver can park
		ACCURATE,  // accurate driver can park
		EXPERIENCED  // only experienced driver can park
	};

	static std::string getComplexityName(PARK_COMPLEXITY _complexity) {
		switch (_complexity) {
			case ACCURATE:
				return "ACCURATE";
			case EXPERIENCED:
				return "EXPERIENCED";
			case NEWBIE:
				return "NEWBIE";
			default:
				return "UNKNOWN";
		}
	}

public:
	ParkingPlace(const GeoCoordinatePoint & _point) : ParkingPlace(_point, UNKNOWN) {}

	ParkingPlace(const GeoCoordinatePoint & _point, PARK_COMPLEXITY _complexity) :
		m_complexity(_complexity), m_point(_point), m_bImageSet(false)
	{}

	bool hasImage() const {return m_bImageSet;}

	void setImage(const cv::Mat & _image) {
		m_image = _image;
		m_bImageSet = true;
	}

	const cv::Mat & getImage() const {
		assert(hasImage());
		return m_image;
	}

	PARK_COMPLEXITY getComplexity() const {return m_complexity;}
	const GeoCoordinatePoint & getPoint() const {return m_point;}

	std::string toString() const {
		std::ostringstream ss;
		ss << std::setprecision(8)
		   << "{ lat: " << m_point.getLatitude()
		   << "; lon: " << m_point.getLongitude()
		   << "; complexity: " << getComplexityName(m_complexity)
		   << " }";
		return ss.str();
	}

private:
	PARK_COMPLEXITY m_complexity;
	GeoCoordinatePoint m_point;

	bool m_bImageSet;
	cv::Mat m_image;
};

#endif // ParkingPlace_H
