#include "GeoCoordinatePoint.h"
#include "MathUtils.h"

namespace {

const unsigned int EARTH_RADIUS_KM = 6371;

} // anonymous namespace

double GeoCoordinatePoint::calculateDistance(const GeoCoordinatePoint & _pt1, const GeoCoordinatePoint & _pt2) {
	const double dDeltaLatitude = MathUtils::degreesToRadians(_pt2.getLatitude() - _pt1.getLatitude());
	const double dDeltaLongitude = MathUtils::degreesToRadians(_pt2.getLongitude() - _pt1.getLongitude());

	const double dBeginLatitude = MathUtils::degreesToRadians(_pt1.getLatitude());
	const double dEndLatitude = MathUtils::degreesToRadians(_pt2.getLatitude());

	const double dA = std::sin(dDeltaLatitude/2) * std::sin(dDeltaLatitude/2) +
		std::sin(dDeltaLongitude/2) * std::sin(dDeltaLongitude/2) * std::cos(dBeginLatitude) * std::cos(dEndLatitude);

	const double dC = 2. * std::atan2(std::sqrt(dA), std::sqrt(1. - dA));
	return dC * EARTH_RADIUS_KM * 1000;
}

double GeoCoordinatePoint::calculateBearing(const GeoCoordinatePoint & _pt1, const GeoCoordinatePoint & _pt2) {
	const double dBeginLatitude = MathUtils::degreesToRadians(_pt1.getLatitude());
	const double dEndLatitude = MathUtils::degreesToRadians(_pt2.getLatitude());
	const double dDeltaLongitude = MathUtils::degreesToRadians(_pt2.getLongitude() - _pt1.getLongitude());

	const double dY = std::sin(dDeltaLongitude) * std::cos(dEndLatitude);
	const double dX = std::cos(dBeginLatitude) * std::sin(dEndLatitude) -
		std::sin(dBeginLatitude) * std::cos(dEndLatitude) * std::cos(dDeltaLongitude);

	return MathUtils::radiansToDegrees(std::atan2(dY, dX));
}

GeoCoordinatePoint GeoCoordinatePoint::calculatePointCoordinates(
	const GeoCoordinatePoint & _startPt,
	double _dBearingInDegrees,
	double _dDistanceInMeters
) {
	const double dStartLatitude = MathUtils::degreesToRadians(_startPt.getLatitude());
	const double dStartLongitude = MathUtils::degreesToRadians(_startPt.getLongitude());
	const double dBearingInRads = MathUtils::degreesToRadians(_dBearingInDegrees);
	const double dAngularDistance = _dDistanceInMeters / (EARTH_RADIUS_KM * 1000);

	const double dLatitude = std::asin(
		std::sin(dStartLatitude) * std::cos(dAngularDistance) +
		std::cos(dStartLatitude) * std::sin(dAngularDistance) * std::cos(dBearingInRads)
	);

	const double dLongitude = dStartLongitude + std::atan2(
		std::sin(dBearingInRads) * std::sin(dAngularDistance) * std::cos(dStartLatitude),
		std::cos(dAngularDistance) - std::sin(dStartLatitude) * std::sin(dLatitude)
	);

	return GeoCoordinatePoint(MathUtils::radiansToDegrees(dLatitude), MathUtils::radiansToDegrees(dLongitude));
}
