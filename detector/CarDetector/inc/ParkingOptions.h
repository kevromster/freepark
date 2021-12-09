#ifndef ParkingOptions_H
#define ParkingOptions_H

#include "GeoCoordinatePoint.h"

#include <opencv2/core/types.hpp>

/**
 * These options define parking place on a camera screenshot.
 */
class ParkingOptions
{
public:
	ParkingOptions();

	long getParkingId() const {return m_lParkingId;}
	int getParkingCapacity() const {return m_nParkingCapacity;}
	bool isUseCameraDistanceMethod() const {return m_bUseCameraDistanceMethod;}

	const std::string & getParkingName() const {return m_strParkingName;}
	const cv::Rect & getParkingImageRectangle() const {return m_parkingImageRectangle;}
	const std::vector<cv::Point> & getParkingImageContour() const {return m_parkingImageContour;}

	const std::vector<cv::Point> & getParkingEdges() const {return m_parkingEdges;}
	const cv::Rect & getParkingEdgesBoundingRectangle() const {return m_parkingEdgesBoundingRect;}

	double getDetectionThreshold() const {return m_dDetectionThreshold;}
	double getRotationDegree() const {return m_dRotationDegree;}

	int getShiftTop() const {return m_nShiftTop;}
	int getShiftBottom() const {return m_nShiftBottom;}

	int getParkingLineDelta() const {return m_nParkingLineDelta;}

	double getParkingLengthInMeters() const {return m_dParkingLengthInMeters;}
	double getParkingBearingInDegrees() const {return m_dParkingBearingInDegrees;}

	const cv::Point & getParkingLineStartPoint() const {return m_parkingLineStartPoint;}
	const cv::Point & getParkingLineEndPoint() const {return m_parkingLineEndPoint;}
	const cv::Point & getParkingLineMidPoint() const {return m_parkingLineMidPoint;}
	const cv::Point & getParkingFarLineStartPoint() const {return m_parkingFarLineStartPoint;}
	const cv::Point & getParkingFarLineEndPoint() const {return m_parkingFarLineEndPoint;}
	const cv::Point & getParkingFarLineMidPoint() const {return m_parkingFarLineMidPoint;}

	double getParkingLineLengthInPixels() const {return m_dParkingLineLengthInPixels;}
	double getParkingFarLineLengthInPixels() const {return m_dParkingFarLineLengthInPixels;}

	double getParkingLineDistortionFactor() const {return m_dParkingLineDistortionFactor;}
	double getParkingFarLineDistortionFactor() const {return m_dParkingFarLineDistortionFactor;}

	double getParkingLineAngle() const {return m_dParkingLineAngle;}
	double getParkingFarLineAngle() const {return m_dParkingFarLineAngle;}

	const GeoCoordinatePoint & getStartPoint() const {return m_startPoint;}
	const GeoCoordinatePoint & getEndPoint() const {return m_endPoint;}

	void setParkingId(long _id) {m_lParkingId = _id;}
	void setParkingCapacity(int _nCapacity) {m_nParkingCapacity = _nCapacity;}

	void setParkingName(const std::string & _strName) {m_strParkingName = _strName;}
	void setDetectionThreshold(double _dThreshold) {m_dDetectionThreshold = _dThreshold;}
	void setRotationDegree(double _dDegree) {m_dRotationDegree = _dDegree;}
	void setUseCameraDistanceMethod(bool value) {m_bUseCameraDistanceMethod = value;}

	void setShiftTop(int _nShift) {m_nShiftTop = _nShift;}
	void setShiftBottom(int _nShift) {m_nShiftBottom = _nShift;}

	void setParkingLineDelta(int _nDelta) {m_nParkingLineDelta = _nDelta;}

	void setParkingImageRectangle(const cv::Rect & _rect) {m_parkingImageRectangle = _rect;}
	void setParkingImageContour(const std::vector<cv::Point> & _points) {m_parkingImageContour = _points;}

	void setParkingLine(const cv::Point & _startPt, const cv::Point & _midPt, const cv::Point & _endPt);
	void setParkingFarLine(const cv::Point & _startPt, const cv::Point & _midPt, const cv::Point & _endPt);

	void setStartPoint(const GeoCoordinatePoint & _pt);
	void setEndPoint(const GeoCoordinatePoint & _pt);

private:
	void _calculateParkingLengthAndBearing();
	void _fillParkingEdges();

private:
	long m_lParkingId;
	int m_nParkingCapacity;

	std::string m_strParkingName;
	cv::Rect m_parkingImageRectangle;             // parking area rectangle onto a captured camera image
	std::vector<cv::Point> m_parkingImageContour; // pixel points specifying parking contour inside a parking rectangle on the parking image

	// detection threshold for this parking
	double m_dDetectionThreshold;

	// counter-clockwise rotation degree of the parking cropped image relative to the camera captured image
	double m_dRotationDegree;

	// shifting in pixels from top and bottom to restrict parking places boundaries
	int m_nShiftTop;
	int m_nShiftBottom;

	// pixel coorinates of the parking line (inside parking rectangle)
	cv::Point m_parkingLineStartPoint;
	cv::Point m_parkingLineEndPoint;

	// pixel coordinates of the far parking line (inside parking rectangle)
	cv::Point m_parkingFarLineStartPoint;
	cv::Point m_parkingFarLineEndPoint;

	// points specifying center in meters of the parking
	cv::Point m_parkingLineMidPoint;
	cv::Point m_parkingFarLineMidPoint;

	GeoCoordinatePoint m_startPoint;  // parking start point along the long side
	GeoCoordinatePoint m_endPoint;  // parking end point along the long side

	int m_nParkingLineDelta;  // pixels

	double m_dParkingLengthInMeters;
	double m_dParkingBearingInDegrees;

	// if true, we use method that calculates distance from camera up to the object
	bool m_bUseCameraDistanceMethod;

	double m_dParkingLineLengthInPixels;
	double m_dParkingFarLineLengthInPixels;

	// distortion factor = second half line length / first half line length
	double m_dParkingLineDistortionFactor;
	double m_dParkingFarLineDistortionFactor;

	// clockwise angle of parking lines relative OX axis in radians
	double m_dParkingLineAngle;
	double m_dParkingFarLineAngle;

	// parking line start point + end point + parking far line start point + end point
	std::vector<cv::Point> m_parkingEdges;
	cv::Rect m_parkingEdgesBoundingRect;
};

#endif // ParkingOptions_H
