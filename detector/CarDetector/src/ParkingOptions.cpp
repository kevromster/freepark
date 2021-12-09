#include "ParkingOptions.h"

#include "GeoCoordinatePoint.h"

#include <opencv2/imgproc/imgproc.hpp>

ParkingOptions::ParkingOptions() :
	m_lParkingId(0),
	m_nParkingCapacity(0),
	m_dDetectionThreshold(0.),
	m_dRotationDegree(0.),
	m_nShiftTop(0),
	m_nShiftBottom(0),
	m_nParkingLineDelta(0),
	m_dParkingLengthInMeters(0.),
	m_dParkingBearingInDegrees(0.),
	m_bUseCameraDistanceMethod(false),
	m_dParkingLineLengthInPixels(0.),
	m_dParkingFarLineLengthInPixels(0.),
	m_dParkingLineDistortionFactor(0.),
	m_dParkingFarLineDistortionFactor(0.),
	m_dParkingLineAngle(0.),
	m_dParkingFarLineAngle(0.)
{}

void ParkingOptions::setParkingLine(const cv::Point & _startPt, const cv::Point & _midPt, const cv::Point & _endPt) {
	m_parkingLineStartPoint = _startPt;
	m_parkingLineMidPoint = _midPt;
	m_parkingLineEndPoint = _endPt;

	const double dFirstHalfLength = cv::norm(_midPt - _startPt);

	m_dParkingLineLengthInPixels = cv::norm(_endPt - _startPt);
	m_dParkingLineDistortionFactor = (m_dParkingLineLengthInPixels - dFirstHalfLength) / dFirstHalfLength;
	m_dParkingLineAngle = std::atan2(_endPt.y - _startPt.y, _endPt.x - _startPt.x);

	_fillParkingEdges();
}

void ParkingOptions::setParkingFarLine(const cv::Point & _startPt, const cv::Point & _midPt, const cv::Point & _endPt) {
	m_parkingFarLineStartPoint = _startPt;
	m_parkingFarLineMidPoint = _midPt;
	m_parkingFarLineEndPoint = _endPt;

	const double dFirstHalfLength = cv::norm(_midPt - _startPt);

	m_dParkingFarLineLengthInPixels = cv::norm(_startPt - _endPt);
	m_dParkingFarLineDistortionFactor = (m_dParkingFarLineLengthInPixels - dFirstHalfLength) / dFirstHalfLength;
	m_dParkingFarLineAngle = std::atan2(_endPt.y - _startPt.y, _endPt.x - _startPt.x);

	_fillParkingEdges();
}

void ParkingOptions::setStartPoint(const GeoCoordinatePoint & _pt) {
	m_startPoint = _pt;
	_calculateParkingLengthAndBearing();
}

void ParkingOptions::setEndPoint(const GeoCoordinatePoint & _pt) {
	m_endPoint = _pt;
	_calculateParkingLengthAndBearing();
}

void ParkingOptions::_calculateParkingLengthAndBearing() {
	m_dParkingLengthInMeters = GeoCoordinatePoint::calculateDistance(m_startPoint, m_endPoint);
	m_dParkingBearingInDegrees = GeoCoordinatePoint::calculateBearing(m_startPoint, m_endPoint);
}

void ParkingOptions::_fillParkingEdges() {
	m_parkingEdges.clear();
	m_parkingEdges.reserve(4);
	m_parkingEdges.push_back(m_parkingFarLineStartPoint);
	m_parkingEdges.push_back(m_parkingFarLineEndPoint);
	m_parkingEdges.push_back(m_parkingLineEndPoint);
	m_parkingEdges.push_back(m_parkingLineStartPoint);

	m_parkingEdgesBoundingRect = cv::boundingRect(m_parkingEdges);
}
