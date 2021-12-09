#ifndef GeoCoordinatePoint_H
#define GeoCoordinatePoint_H

/**
 * Represents geographical coordinate point.
 */
class GeoCoordinatePoint
{
public:
	/**
	 *  Returns distance in meters between two coordinate points.
	 */
	static double calculateDistance(const GeoCoordinatePoint & _pt1, const GeoCoordinatePoint & _pt2);

	/**
	 * Calculates bearing in degrees from the first point to the second one.
	 */
	static double calculateBearing(const GeoCoordinatePoint & _pt1, const GeoCoordinatePoint & _pt2);

	/**
	 * Calculates coordinates of a point located on the given distance and bearing from the start point.
	 */
	static GeoCoordinatePoint calculatePointCoordinates(
		const GeoCoordinatePoint & _startPt, double _dBearingInDegrees, double _dDistanceInMeters
	);

public:
	GeoCoordinatePoint() :
		m_dLatitude(0.),
		m_dLongitude(0.)
	{}

	GeoCoordinatePoint(double _dLatitudeInDegrees, double _dLongitudeInDegrees) :
		m_dLatitude(_dLatitudeInDegrees),
		m_dLongitude(_dLongitudeInDegrees)
	{}

	double getLatitude() const {return m_dLatitude;}
	double getLongitude() const {return m_dLongitude;}

private:
	double m_dLatitude;
	double m_dLongitude;
};

#endif // GeoCoordinatePoint_H
