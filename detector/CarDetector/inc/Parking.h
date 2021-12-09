#ifndef Parking_H
#define Parking_H

#include "ParkingOptions.h"
#include "ParkingPlace.h"

#include <TFDetectedObjects.h>

#include <memory>

class ITFDetector;

/**
 * Represents parking.
 */
class Parking
{
public:
	Parking(const ParkingOptions & _options);

	/**
	 * Detects free parking places on an image specified via {@link #setParkingImage} method using the given detector.
	 * @param _detector the detector to use
	 * @param _lClassId the class Id of Car objects
	 */
	void detectFreeParkingPlaces(ITFDetector & _detector, long _lCarClassId);

	/**
	 * Sets parking image to be processed during free parking places detection.
	 *
	 * @param _image the image on which to run the detection
	 * @param _luImageId unique image id to identify it
	 */
	void setParkingImage(const cv::Mat & _image, unsigned long _luImageId);

	/**
	 * Returns unique identifier of the parking under which it is be stored on backend server.
	 * @return the parking identifier
	 */
	long getParkingId() const {return m_options.getParkingId();}

	/**
	 * Returns parking image rectangle on the big input image from camera.
	 * @return the parking rectangle
	 */
	const cv::Rect & getParkingImageRectangle() const {return m_options.getParkingImageRectangle();}

	/**
	 * Returns detected free parking places.
	 * @return the vector of free parking places
	 */
	const std::vector<ParkingPlace> & getFreeParkingPlaces() const {return m_freeParkingPlaces;}

private:
	void _initImageToDetectOn();
	cv::Mat _createImageToDetectOn() const;
	const cv::Mat & _getImageToDetectOn() const;

	void _runDetection(ITFDetector & _detector, long _lCarClassId);

	void _findFreeParkingPlaces();
	void _findFreeParkingPlacesForTopCameraView();
	void _findFreeParkingPlacesUsingCameraDistanceMethod();

	// Converts pixels from image's top to the real distance in meters.
	double _pixelsToDistance(unsigned int _uPixelsFromTop) const;

	unsigned int _getDetectionImageHeight() const;
	unsigned int _getDetectionImageWidth() const;

	bool _isParkingImageHasRotation() const;
	unsigned int _getDefaultCarWidth() const;
	cv::Rect _getMiddleParkingPlacesBoundaries() const;

	void _checkForFreeSpace(
		const TFDetectedObject * _pObject,
		const TFDetectedObject * _pPrevObject,
		unsigned int _uCarWidth,
		const cv::Rect & _middleBoundaries
	);

	// for UseCameraDistance method
	void _checkForFreeSpaceMeters(
		const TFDetectedObject * _pObject,
		const TFDetectedObject * _pPrevObject,
		double _dCarWidthMeters
	);

	// calculates distance in meters along the parking line from its start point to the edge of the object for UseCameraDistance method
	double _calculateDistanceToRightEdge(const TFDetectedObject & _object) const;
	double _calculateDistanceToLeftEdge(const TFDetectedObject & _object, double _dCarWidth) const;

	double _calculateRangedDistanceToRightEdge(const TFDetectedObject & _object) const;
	double _calculateRangedDistanceToLeftEdge(const TFDetectedObject & _object, double _dCarWidth) const;

	double _calculateDistanceByParkingLine(double _dXpixel, bool _bUseParkingDeltaCorrection) const;
	double _calculateDistanceByParkingLine(int _nXpixel) const;
	double _calculateDistanceByParkingFarLine(int _nXpixel) const;

	// returns point on the parking line, linking line from which goes through the given input point
	cv::Point2d _projectPointToParkingLine(const cv::Point & _point) const;
	cv::Point2d _projectPointToParkingLine(const cv::Point & _point, double _dProjectionAngle) const;

	bool _isPointUpperThanParkingLine(const cv::Point & _point) const;
	int _getXpixelProjectedToParkingLine(const cv::Point & _point) const;

	// returns point on the far parking line that corresponds to the given point on the parking line
	cv::Point2d _projectParkingLinePointToFarLinePoint(const cv::Point2d & _point) const;

	unsigned int _calculateParkPlaceWidth(unsigned int _uCarWidth, ParkingPlace::PARK_COMPLEXITY _complexity) const;
	double _calculateParkPlaceWidthD(double _dCarWidth, ParkingPlace::PARK_COMPLEXITY _complexity) const;

	unsigned int _calculateFittingCarsCount(
		int nSpace,
		unsigned int _uCarWidth,
		ParkingPlace::PARK_COMPLEXITY _complexity
	) const;

	unsigned int _calculateFittingCarsCountD(
		double _dSpace,
		double _dCarWidth,
		ParkingPlace::PARK_COMPLEXITY _complexity
	) const;

	void _saveFreeParkingPlaceD(
		double _dDistanceToPrevObjectRightEdge,
		double _dDistanceToObjectLeftEdge,
		double _dPlaceWidth,
		ParkingPlace::PARK_COMPLEXITY _complexity
	);

	void _saveFreeParkingPlace(
		unsigned int _uTopBoundary,
		unsigned int _uBottomBoundary,
		unsigned int _uPlaceWidth,
		ParkingPlace::PARK_COMPLEXITY _complexity,
		const cv::Rect & _middleBoundaries
	);

	cv::Mat _createParkingPlaceImage(
		unsigned int _uTopShift,
		unsigned int _uWidth,
		ParkingPlace::PARK_COMPLEXITY _complexity,
		const cv::Rect & _middleBoundaries
	) const;

	cv::Mat _createParkingPlaceImageD(
		double _dObj1,
		double _dObj2,
		ParkingPlace::PARK_COMPLEXITY _complexity
	) const;

	void _saveDetectionImages();

	void _drawBBoxes(cv::Mat & _image) const;
	void _drawParkingEdges(cv::Mat & _image) const;

	// draws connecting line between close and far lines of the parking on the specified distance (in meters) from the parking edge
	void _drawLinkingLine(cv::Mat & _image, double _distance, const cv::Scalar & _color, int thickness) const;
	void _drawLinkingLineWithText(
		cv::Mat & _image,
		double _distance,
		const cv::Scalar & _color,
		int thickness,
		const std::string & _text,
		bool _bIsTextNearFarLine
	) const;

#ifdef SAVE_DEBUG_IMAGES
	void _saveDebugImage(const cv::Mat & _image, const std::string & _strPrefix, const std::string & _strPostfix) const;
#endif

	bool _isObjectIntersectsWithParkingArea(const TFDetectedObject & _o) const;
	bool _isFirstCloserToParkingLineStartPointThanSecond(const TFDetectedObject & _o1, const TFDetectedObject & _o2) const;

private:
	const ParkingOptions m_options;
	std::unique_ptr<TFDetectedObjects> m_pDetectedObjects;

	cv::Mat m_lastParkingImage;
	std::unique_ptr<cv::Mat> m_pParkingImageToDetectOn;  // parking image with initial rotation or other preparation done
	unsigned long m_luLastImageId;

	std::vector<ParkingPlace> m_freeParkingPlaces;

#ifdef SAVE_DEBUG_IMAGES
	cv::Mat m_debugDrawImage;
#endif
};

#endif // Parking_H
