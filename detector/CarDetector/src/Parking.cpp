#include "Parking.h"
#include "CarDetectorLog.h"
#include "MathUtils.h"

#include <ITFDetector.h>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/viz/types.hpp>

namespace {

// Reserve to fit car into the detected place (in percents from typical car width)
const int CAR_FITTING_RESERVE_PERCENTS_NEWBIE = 0;
const int CAR_FITTING_RESERVE_PERCENTS_ACCURATE = -5;
const int CAR_FITTING_RESERVE_PERCENTS_EXPERIENCED = -10;

// to calculate real car width decrease by this percent
const int CAR_FITTING_REAL_WIDTH_PERCENTS = CAR_FITTING_RESERVE_PERCENTS_EXPERIENCED - 5;

#ifdef SAVE_DEBUG_IMAGES
const cv::Scalar DEBUG_IMAGE_PARKING_EDGES_COLOR = cv::viz::Color::yellow();
const cv::Scalar DEBUG_IMAGE_OBJECT_LEFT_EDGE_COLOR = cv::viz::Color::blue();
const cv::Scalar DEBUG_IMAGE_OBJECT_RIGHT_EDGE_COLOR = cv::viz::Color::red();
#endif // SAVE_DEBUG_IMAGES

int getCarFittingReserve(ParkingPlace::PARK_COMPLEXITY _complexity) {
	switch (_complexity) {
		case ParkingPlace::ACCURATE:
			return CAR_FITTING_RESERVE_PERCENTS_ACCURATE;
		case ParkingPlace::EXPERIENCED:
			return CAR_FITTING_RESERVE_PERCENTS_EXPERIENCED;
		case ParkingPlace::NEWBIE:
		default:
			return CAR_FITTING_RESERVE_PERCENTS_NEWBIE;
	}
}

cv::Scalar getComplexityColor(ParkingPlace::PARK_COMPLEXITY _complexity) {
	switch (_complexity) {
		case ParkingPlace::ACCURATE:
			return CV_RGB(255, 255, 0);
		case ParkingPlace::NEWBIE:
			return CV_RGB(0, 255, 0);
		case ParkingPlace::EXPERIENCED:
		default:
			return CV_RGB(255, 0, 0);
	}
}

int half(int nValue) {
	return static_cast<int>(std::lround(nValue / 2.0));
}

// Determines whether the first bbox lies upper by OY axis than the second one.
bool isFirstUpperThanSecond(const TFDetectedObject & _o1, const TFDetectedObject & _o2) {
	const cv::Rect & bbox1 = _o1.getBBox();
	const cv::Rect & bbox2 = _o2.getBBox();

	if (bbox2.y - (bbox1.y + bbox1.height) >= 0) {
		// the first bbox is upper than the second one
		return true;
	}

	if (bbox1.y - (bbox2.y + bbox2.height) >= 0) {
		// the first bbox is lower than the second one
		return false;
	}

	// boxes intersect, use box centers to compare
	int spaceBetweenCenters = (bbox2.y + half(bbox2.height)) - (bbox1.y + half(bbox1.height));
	return spaceBetweenCenters >= 0;
}

int inRange(int nValue, int nFrom, int nTo) {
	return std::max(nFrom, std::min(nTo, nValue));
}

double inRange(double dValue, double dFrom, double dTo) {
	return std::max(dFrom, std::min(dTo, dValue));
}

cv::Mat rotateImage(const cv::Mat & _image, double _degree) {
	if (std::abs(_degree) <= MathUtils::EPSILON)
		return _image;

	cv::Point2d center(_image.cols/2.0, _image.rows/2.0);
	cv::Mat rotationMatrix = cv::getRotationMatrix2D(center, _degree, 1.0);

	cv::Rect bbox = cv::RotatedRect(center, _image.size(), static_cast<float>(_degree)).boundingRect();
	rotationMatrix.at<double>(0,2) += bbox.width/2.0 - center.x;
	rotationMatrix.at<double>(1,2) += bbox.height/2.0 - center.y;

	cv::Mat rotatedImage;
	cv::warpAffine(_image, rotatedImage, rotationMatrix, bbox.size());
	return rotatedImage;
}

cv::Mat clipByContour(const cv::Mat & _image, const std::vector<cv::Point> & _contour) {
	assert(_contour.size() >= 3);
	std::vector<std::vector<cv::Point>> contours;
	contours.push_back(_contour);

	cv::Mat mask = cv::Mat::zeros(_image.rows, _image.cols, CV_8UC1);
	cv::drawContours(mask, contours, -1, cv::Scalar(255), CV_FILLED);

	cv::Mat clipped(_image.rows, _image.cols, CV_8UC3);
	_image.copyTo(clipped, mask);

	return clipped;
}

// calculates distance in meters by the given distance in pixels
double calcDistanceMeters(double _dPixels, double W, double L, double K) {
	if (std::abs(_dPixels) < MathUtils::EPSILON)
		return 0.;
	return (L * K) / ((W / _dPixels) - 1. + K);
}

// calculates distance in pixels by the given distance in meters
double calcDistancePixels(double _dMeters, double W, double L, double K) {
	if (std::abs(_dMeters) < MathUtils::EPSILON)
		return 0.;
	return W / (((L * K) / _dMeters) + 1. - K);
}

} // anonymous namespace

Parking::Parking(const ParkingOptions & _options) :
	m_options(_options),
	m_luLastImageId(0)
{
}

void Parking::setParkingImage(const cv::Mat & _image, unsigned long _luImageId) {
	_image.copyTo(m_lastParkingImage);
	m_luLastImageId = _luImageId;
	m_pParkingImageToDetectOn.reset();

#ifdef SAVE_DEBUG_IMAGES
	m_debugDrawImage = _image.clone();
#endif
}

unsigned int Parking::_getDetectionImageHeight() const {
	return static_cast<unsigned int>(std::max(_getImageToDetectOn().size().height, 0));
}

unsigned int Parking::_getDetectionImageWidth() const {
	return static_cast<unsigned int>(std::max(_getImageToDetectOn().size().width, 0));
}

double Parking::_pixelsToDistance(unsigned int _uPixelsFromTop) const {
	return (m_options.getParkingLengthInMeters() * _uPixelsFromTop) / _getDetectionImageHeight();
}

void Parking::detectFreeParkingPlaces(ITFDetector & _detector, long _lCarClassId) {
	HDLOG << "Running detection for parking " << m_options.getParkingName() << "...";
	_initImageToDetectOn();
	_runDetection(_detector, _lCarClassId);
}

void Parking::_runDetection(ITFDetector & _detector, long _lCarClassId) {
	std::vector<long> classIds;
	classIds.reserve(1);
	classIds.push_back(_lCarClassId);

	m_pDetectedObjects = _detector.detectObjects(_getImageToDetectOn(), m_options.getDetectionThreshold(), classIds);

	_findFreeParkingPlaces();
	_saveDetectionImages();
}

void Parking::_findFreeParkingPlaces() {
	m_freeParkingPlaces.clear();
	HDLOG << "Searching for fresh free parking places for image id " << m_luLastImageId << "...";

	if (m_options.isUseCameraDistanceMethod())
		_findFreeParkingPlacesUsingCameraDistanceMethod();
	else
		_findFreeParkingPlacesForTopCameraView();
}

double Parking::_calculateDistanceByParkingFarLine(int _nXpixel) const {
	const double W = m_options.getParkingFarLineLengthInPixels();
	const double K = m_options.getParkingFarLineDistortionFactor();
	const double L = m_options.getParkingLengthInMeters();

	const double dAngleRadians = m_options.getParkingFarLineAngle();
	const double distanceX = _nXpixel - m_options.getParkingFarLineStartPoint().x;

	const double cameraDistancePixels = distanceX / std::cos(dAngleRadians);
	const double cameraDistanceMeters = calcDistanceMeters(cameraDistancePixels, W, L, K);

	return cameraDistanceMeters;
}

double Parking::_calculateDistanceByParkingLine(int _nXpixel) const {
	return _calculateDistanceByParkingLine(_nXpixel, true);
}

double Parking::_calculateDistanceByParkingLine(double _dXpixel, bool _bUseParkingDeltaCorrection) const {
	const double W = m_options.getParkingLineLengthInPixels();
	const double K = m_options.getParkingLineDistortionFactor();
	const double L = m_options.getParkingLengthInMeters();

	const double dAngleRadians = m_options.getParkingLineAngle();
	const double distanceX =  _dXpixel - m_options.getParkingLineStartPoint().x;

	const double cameraDistancePixels = distanceX / std::cos(dAngleRadians);
	double cameraDistanceMeters = calcDistanceMeters(cameraDistancePixels, W, L, K);

	if (_bUseParkingDeltaCorrection && m_options.getParkingLineDelta() > 0) {
		const double Wfar = m_options.getParkingFarLineLengthInPixels();
		const double Kfar = m_options.getParkingFarLineDistortionFactor();
		const double farLineAngle = m_options.getParkingFarLineAngle();
		const double farLinePixels = calcDistancePixels(cameraDistanceMeters, Wfar, L, Kfar);

		const cv::Point & parkingLineStartPoint = m_options.getParkingLineStartPoint();
		const cv::Point & parkingFarLineStartPoint = m_options.getParkingFarLineStartPoint();

		const double Xfar = parkingFarLineStartPoint.x + farLinePixels * std::cos(farLineAngle);
		const double Yfar = parkingFarLineStartPoint.y + farLinePixels * std::sin(farLineAngle);
		const double Xclose = parkingLineStartPoint.x + cameraDistancePixels * std::cos(dAngleRadians);
		const double Yclose = parkingLineStartPoint.y + cameraDistancePixels * std::sin(dAngleRadians);

		const double Xdelta = std::abs(Xfar - Xclose);
		const double Ydelta = std::abs(Yfar - Yclose);

		const double H = m_options.getParkingLineDelta();

		const double sinAlpha = Xdelta / std::sqrt(Xdelta*Xdelta + Ydelta*Ydelta);
		const double cameraDistanceDelta = H * sinAlpha / std::cos(dAngleRadians);

		HDLOG << "correcting distance to right edge by " << cameraDistanceDelta << " pixels...";

		// correct by found delta
		cameraDistanceMeters = calcDistanceMeters(cameraDistancePixels + cameraDistanceDelta, W, L, K);
	}

	return cameraDistanceMeters;
}

double Parking::_calculateDistanceToLeftEdge(const TFDetectedObject & _object, double _dCarWidth) const {
	const double dRealCarWidth = _dCarWidth * (1. + CAR_FITTING_REAL_WIDTH_PERCENTS/100.);
	const double distanceToRightEdge = _calculateDistanceToRightEdge(_object);
	return distanceToRightEdge - dRealCarWidth;
}

double Parking::_calculateRangedDistanceToLeftEdge(const TFDetectedObject & _object, double _dCarWidth) const {
	return inRange(_calculateDistanceToLeftEdge(_object, _dCarWidth), 0., m_options.getParkingLengthInMeters());
}

double Parking::_calculateRangedDistanceToRightEdge(const TFDetectedObject & _object) const {
	return inRange(_calculateDistanceToRightEdge(_object), 0., m_options.getParkingLengthInMeters());
}

double Parking::_calculateDistanceToRightEdge(const TFDetectedObject & _object) const {
	const cv::Rect & bbox = _object.getBBox();
	const cv::Point bottomRightPoint = bbox.br();
	int nXpixel = _getXpixelProjectedToParkingLine(bottomRightPoint);

	return _calculateDistanceByParkingLine(nXpixel);
}

int Parking::_getXpixelProjectedToParkingLine(const cv::Point & _point) const {
	int nXpixel = _point.x;

	// we project the point only if it is upper than the parking line, otherwise just use its X coordinate (i.e. project vertically)
	if (_isPointUpperThanParkingLine(_point)) {
		const cv::Point projectedPointOnParkingLine = _projectPointToParkingLine(_point);
		nXpixel = projectedPointOnParkingLine.x;
	}

	return nXpixel;
}

bool Parking::_isPointUpperThanParkingLine(const cv::Point & _point) const {
	const double distanceX = _point.x - m_options.getParkingLineStartPoint().x;
	const double distanceY = distanceX * std::tan(m_options.getParkingLineAngle());
	const double pointOnLineYcoord = m_options.getParkingLineStartPoint().y + distanceY;

	return pointOnLineYcoord > _point.y;
}

cv::Point2d Parking::_projectPointToParkingLine(const cv::Point & _point) const {
	// General algorithm:
	// - project by 90 degrees;
	// - get linking line angle for the projected point;
	// - reproject again for the found angle;
	// - get linking line angle for the projected point;
	// - ... reproject again and again until diff in found angle less than epsilon.

	// As first step directly project the point by 90 degrees for the parking line.
	// The projection angle from the vertical line will be equal to the parking line angle in this case.
	return _projectPointToParkingLine(_point, -m_options.getParkingLineAngle());
}

cv::Point2d Parking::_projectPointToParkingLine(const cv::Point & _point, double _dProjectionAngle) const {
	const cv::Point2d projectedPointOnParkingLine =
		MathUtils::projectPointToLine(_point, _dProjectionAngle, m_options.getParkingLineStartPoint(), -m_options.getParkingLineAngle());

	const cv::Point2d projectedPointOnParkingFarLine = _projectParkingLinePointToFarLinePoint(projectedPointOnParkingLine);

	double dLinkingLineAngle =
		std::atan2(projectedPointOnParkingLine.x - projectedPointOnParkingFarLine.x,
				   projectedPointOnParkingLine.y - projectedPointOnParkingFarLine.y);

	if (std::abs(dLinkingLineAngle - _dProjectionAngle) <= MathUtils::EPSILON)
		return projectedPointOnParkingLine;

	// reproject again by the new angle
	return _projectPointToParkingLine(_point, dLinkingLineAngle);
}

cv::Point2d Parking::_projectParkingLinePointToFarLinePoint(const cv::Point2d & _point) const {
	const double dDistanceMeters = _calculateDistanceByParkingLine(_point.x, false);

	const double W = m_options.getParkingFarLineLengthInPixels();
	const double K = m_options.getParkingFarLineDistortionFactor();
	const double L = m_options.getParkingLengthInMeters();

	const double dAngle = m_options.getParkingFarLineAngle();
	const double dDistancePixelsByFarLine = calcDistancePixels(dDistanceMeters, W, L, K);

	const double Xdelta = dDistancePixelsByFarLine * std::cos(dAngle);
	const double Ydelta = dDistancePixelsByFarLine * std::sin(dAngle);

	const cv::Point & startPoint = m_options.getParkingFarLineStartPoint();
	return cv::Point2d(startPoint.x + Xdelta, startPoint.y + Ydelta);
}

bool Parking::_isObjectIntersectsWithParkingArea(const TFDetectedObject & _o) const {
	// intersect the bbox with parking edges and calc got rect area
	return (_o.getBBox() & m_options.getParkingEdgesBoundingRectangle()).area() > 0;
}

bool Parking::_isFirstCloserToParkingLineStartPointThanSecond(const TFDetectedObject & _o1, const TFDetectedObject & _o2) const {
	return _calculateDistanceToRightEdge(_o1) < _calculateDistanceToRightEdge(_o2);
}

void Parking::_findFreeParkingPlacesUsingCameraDistanceMethod() {
	HDLOG << "Using UseCameraDistance algorithm...";
	std::function<bool(const TFDetectedObject&, const TFDetectedObject&)> compareFunc =
		[this](const TFDetectedObject & _o1, const TFDetectedObject& _o2) { return this->_isFirstCloserToParkingLineStartPointThanSecond(_o1, _o2); };

	m_pDetectedObjects->sortDetectedObjects(compareFunc);

#ifdef SAVE_DEBUG_IMAGES
	// draw edges at the beginning in order to draw later all found information above them
	_drawParkingEdges(m_debugDrawImage);
#endif

	const double dCarWidthInMeters = m_options.getParkingLengthInMeters() / m_options.getParkingCapacity();
	HDLOG << "Parking length: " << m_options.getParkingLengthInMeters() << " meters";
	HDLOG << "Set car width to " << dCarWidthInMeters << " meters";

	const TFDetectedObject * pPrevObject = nullptr;
	for (const TFDetectedObject & object : m_pDetectedObjects->getDetectedObjects()) {
		// skip objects outside of the parking area
		if (_isObjectIntersectsWithParkingArea(object)) {
			_checkForFreeSpaceMeters(&object, pPrevObject, dCarWidthInMeters);
			pPrevObject = &object;
		}
	}

	_checkForFreeSpaceMeters(nullptr, pPrevObject, dCarWidthInMeters);
}

void Parking::_findFreeParkingPlacesForTopCameraView() {
	// We assume that detected rectangles are aligned along the long side of the parking.
	// So process them from top to bottom calculating free space between bboxes.

	HDLOG << "Using TopCameraView algorithm...";

	std::function<bool(const TFDetectedObject&, const TFDetectedObject&)> compareFunc = isFirstUpperThanSecond;
	m_pDetectedObjects->sortDetectedObjects(compareFunc);

	const unsigned int uCarWidth = _getDefaultCarWidth();
	HDLOG << "Set car width to " << uCarWidth;

	const cv::Rect middleBoundaries = _getMiddleParkingPlacesBoundaries();

	const TFDetectedObject * pPrevObject = nullptr;
	for (const TFDetectedObject & object : m_pDetectedObjects->getDetectedObjects()) {
		_checkForFreeSpace(&object, pPrevObject, uCarWidth, middleBoundaries);
		pPrevObject = &object;
	}

	// check the last bbox with the image end
	_checkForFreeSpace(nullptr, pPrevObject, uCarWidth, middleBoundaries);
}

void Parking::_checkForFreeSpaceMeters(
	const TFDetectedObject * _pObject,
	const TFDetectedObject * _pPrevObject,
	double _dCarWidthMeters
) {
	// TODO: should use Top/Bottom edges to calculate space if parking line angle is greater than PI/2.

	const double dDistanceToObjectLeftEdge = _pObject ? _calculateRangedDistanceToLeftEdge(*_pObject, _dCarWidthMeters) : m_options.getParkingLengthInMeters();
	const double dDistanceToPrevObjectRightEdge = _pPrevObject ? _calculateRangedDistanceToRightEdge(*_pPrevObject) : 0.;
	const double dSpace = dDistanceToObjectLeftEdge - dDistanceToPrevObjectRightEdge;

	const std::string strFirstObject = (_pPrevObject ? "id" + std::to_string(_pPrevObject->getId()) : "BEGIN");
	const std::string strSecondObject = (_pObject ? "id" + std::to_string(_pObject->getId()) : "END");

#ifdef SAVE_DEBUG_IMAGES
	_drawLinkingLineWithText(m_debugDrawImage, dDistanceToPrevObjectRightEdge, DEBUG_IMAGE_OBJECT_RIGHT_EDGE_COLOR, 2, _pPrevObject ? strFirstObject : "B", false);
	_drawLinkingLineWithText(m_debugDrawImage, dDistanceToObjectLeftEdge, DEBUG_IMAGE_OBJECT_LEFT_EDGE_COLOR, 2, _pObject ? strSecondObject : "E", true);
#endif // SAVE_DEBUG_IMAGES

	HDLOG << "Space between objects <" << strFirstObject << "," << strSecondObject << ">: " << dSpace << " meters";

	// TODO: unify with another similar method

	// take minimal reserve first
	ParkingPlace::PARK_COMPLEXITY calcComplexity = ParkingPlace::EXPERIENCED;
	const unsigned int uCarsCount = _calculateFittingCarsCountD(dSpace, _dCarWidthMeters, calcComplexity);

	if (uCarsCount > 0) {
		if (uCarsCount == 1) {
			// One parking place found, check its real complexity.
			if (_calculateFittingCarsCountD(dSpace, _dCarWidthMeters, ParkingPlace::NEWBIE) > 0)
				calcComplexity = ParkingPlace::NEWBIE;
			else if (_calculateFittingCarsCountD(dSpace, _dCarWidthMeters, ParkingPlace::ACCURATE) > 0)
				calcComplexity = ParkingPlace::ACCURATE;
			else
				calcComplexity = ParkingPlace::EXPERIENCED;
		}

		// If more than one places found, complexity is not really important, so set NEWBIE to all places.
		ParkingPlace::PARK_COMPLEXITY complexityToSet = uCarsCount > 1 ? ParkingPlace::NEWBIE : calcComplexity;

		HDLOG
			<< "Found free place " << dSpace << " meters between objects <"
			<< strFirstObject << "," << strSecondObject
			<< "> for cars count: " << uCarsCount
			<< "; calced complexity: " << ParkingPlace::getComplexityName(calcComplexity)
			<< "; complexity to set: " << ParkingPlace::getComplexityName(complexityToSet)
			<< "; input image id: " << m_luLastImageId;

		_saveFreeParkingPlaceD(
			dDistanceToPrevObjectRightEdge,
			dDistanceToObjectLeftEdge,
			uCarsCount == 1 ? dSpace : _calculateParkPlaceWidthD(_dCarWidthMeters, calcComplexity),
			complexityToSet
		);
	}
}

void Parking::_checkForFreeSpace(
	const TFDetectedObject * _pObject,
	const TFDetectedObject * _pPrevObject,
	unsigned int _uCarWidth,
	const cv::Rect & _middleBoundaries
) {
	const int nDetectionImageHeight = static_cast<int>(_getDetectionImageHeight());

	const int nBottomBoundary =
		inRange(
			_pObject ? _pObject->getBBox().y : nDetectionImageHeight,
			0 + m_options.getShiftTop(),
			nDetectionImageHeight - m_options.getShiftBottom()
		);

	const int nTopBoundary =
		inRange(
			_pPrevObject ? _pPrevObject->getBBox().y + _pPrevObject->getBBox().height : 0,
			0 + m_options.getShiftTop(),
			nDetectionImageHeight - m_options.getShiftBottom()
		);

	const int nSpace = nBottomBoundary - nTopBoundary;

	const std::string strFirstObject = (_pPrevObject ? "id" + std::to_string(_pPrevObject->getId()) : "BEGIN");
	const std::string strSecondObject = (_pObject ? "id" + std::to_string(_pObject->getId()) : "END");

	HDLOG << "Space between objects <" << strFirstObject << "," << strSecondObject << ">: " << nSpace;

	// take minimal reserve first
	ParkingPlace::PARK_COMPLEXITY calcComplexity = ParkingPlace::EXPERIENCED;
	const unsigned int uCarsCount = _calculateFittingCarsCount(nSpace, _uCarWidth, calcComplexity);

	if (uCarsCount > 0) {
		if (uCarsCount == 1) {
			// One parking place found, check its real complexity.
			if (_calculateFittingCarsCount(nSpace, _uCarWidth, ParkingPlace::NEWBIE) > 0)
				calcComplexity = ParkingPlace::NEWBIE;
			else if (_calculateFittingCarsCount(nSpace, _uCarWidth, ParkingPlace::ACCURATE) > 0)
				calcComplexity = ParkingPlace::ACCURATE;
			else
				calcComplexity = ParkingPlace::EXPERIENCED;
		}

		// If more than one places found, complexity is not really important, so set NEWBIE to all places.
		ParkingPlace::PARK_COMPLEXITY complexityToSet = uCarsCount > 1 ? ParkingPlace::NEWBIE : calcComplexity;

		HDLOG
			<< "Found free place " << nSpace << " between objects <"
			<< strFirstObject << "," << strSecondObject
			<< "> for cars count: " << uCarsCount
			<< "; calced complexity: " << ParkingPlace::getComplexityName(calcComplexity)
			<< "; complexity to set: " << ParkingPlace::getComplexityName(complexityToSet)
			<< "; input image id: " << m_luLastImageId;

		assert(nBottomBoundary >= 0);
		assert(nTopBoundary >= 0);
		assert(nSpace >= 0);

		_saveFreeParkingPlace(
			static_cast<unsigned int>(nTopBoundary),
			static_cast<unsigned int>(nBottomBoundary),
			uCarsCount == 1 ? static_cast<unsigned int>(nSpace) : _calculateParkPlaceWidth(_uCarWidth, calcComplexity),
			complexityToSet,
			_middleBoundaries
		);
	}
}

double Parking::_calculateParkPlaceWidthD(double _dCarWidth, ParkingPlace::PARK_COMPLEXITY _complexity) const {
	return _dCarWidth * (1. + getCarFittingReserve(_complexity)/100.);
}

unsigned int Parking::_calculateParkPlaceWidth(unsigned int _uCarWidth, ParkingPlace::PARK_COMPLEXITY _complexity) const {
	return static_cast<unsigned int>(std::lround(_uCarWidth * (1. + getCarFittingReserve(_complexity)/100.)));
}

unsigned int Parking::_calculateFittingCarsCountD(
	double _dSpace,
	double _dCarWidth,
	ParkingPlace::PARK_COMPLEXITY _complexity
) const {
	if (_dSpace <= 0.)
		return 0;
	const double dPlaceWidth = _calculateParkPlaceWidthD(_dCarWidth, _complexity);
	return static_cast<unsigned int>(std::lround((_dSpace - std::fmod(_dSpace, dPlaceWidth))/dPlaceWidth));
}

unsigned int Parking::_calculateFittingCarsCount(
	int nSpace,
	unsigned int _uCarWidth,
	ParkingPlace::PARK_COMPLEXITY _complexity
) const {
	if (nSpace <= 0)
		return 0;
	const unsigned int uPlaceWidth = _calculateParkPlaceWidth(_uCarWidth, _complexity);
	const unsigned int uSpace = static_cast<unsigned int>(nSpace);
	return static_cast<unsigned int>(std::lround(static_cast<double>(uSpace - uSpace % uPlaceWidth)/uPlaceWidth));
}

void Parking::_saveFreeParkingPlaceD(
	double _dDistanceToPrevObjectRightEdge,
	double _dDistanceToObjectLeftEdge,
	double _dPlaceWidth,
	ParkingPlace::PARK_COMPLEXITY _complexity
) {
	const double dHalfCarWidth = _dPlaceWidth / 2.;

	for (double dPlaceBegin = _dDistanceToPrevObjectRightEdge;
		 dPlaceBegin + _dPlaceWidth <= _dDistanceToObjectLeftEdge;
		 dPlaceBegin += _dPlaceWidth
	) {
		// assume the start point is at the left edge of the image
		const double dDistanceFromStartPoint = dPlaceBegin + dHalfCarWidth;

		const GeoCoordinatePoint parkingPlaceMiddlePoint = GeoCoordinatePoint::calculatePointCoordinates(
			m_options.getStartPoint(), m_options.getParkingBearingInDegrees(), dDistanceFromStartPoint);

		m_freeParkingPlaces.emplace_back(parkingPlaceMiddlePoint, _complexity);
		m_freeParkingPlaces.back().setImage(_createParkingPlaceImageD(dPlaceBegin, dPlaceBegin + _dPlaceWidth, _complexity));
	}
}

void Parking::_saveFreeParkingPlace(
	unsigned int _uTopBoundary,
	unsigned int _uBottomBoundary,
	unsigned int _uPlaceWidth,
	ParkingPlace::PARK_COMPLEXITY _complexity,
	const cv::Rect & _middleBoundaries
) {
	const unsigned int uHalfCarWidth = static_cast<unsigned int>(std::lround(_uPlaceWidth / 2.));

	for (unsigned int uPlaceBegin = _uTopBoundary;
		 uPlaceBegin + _uPlaceWidth <= _uBottomBoundary;
		 uPlaceBegin += _uPlaceWidth
	) {
		const double dDistanceFromTop = _pixelsToDistance(uPlaceBegin + uHalfCarWidth);

		m_freeParkingPlaces.emplace_back(
			GeoCoordinatePoint::calculatePointCoordinates(
				m_options.getStartPoint(), m_options.getParkingBearingInDegrees(), dDistanceFromTop), _complexity);

		m_freeParkingPlaces.back().setImage(_createParkingPlaceImage(uPlaceBegin, _uPlaceWidth, _complexity, _middleBoundaries));
		HDLOG << "Saved ParkingPlace: " << m_freeParkingPlaces.back().toString();
	}
}

cv::Mat Parking::_createParkingPlaceImageD(
	double _dObj1,
	double _dObj2,
	ParkingPlace::PARK_COMPLEXITY _complexity
) const {
	const double W = m_options.getParkingLineLengthInPixels();
	const double K = m_options.getParkingLineDistortionFactor();
	const double L = m_options.getParkingLengthInMeters();
	const double dAngle = m_options.getParkingLineAngle();

	const cv::Point & startPoint = m_options.getParkingLineStartPoint();

	const double obj1pixels = calcDistancePixels(_dObj1, W, L, K);
	const double obj2pixels = calcDistancePixels(_dObj2, W, L, K);

	const double X1 = startPoint.x + obj1pixels * std::cos(dAngle);
	const double X2 = startPoint.x + obj2pixels * std::cos(dAngle);

	const double Y1 = startPoint.y + obj1pixels * std::sin(dAngle);
	const double Y2 = startPoint.y + obj2pixels * std::sin(dAngle);

	// draw fat line between two points
	cv::Mat image;
	m_lastParkingImage.copyTo(image);

	static int cnt = 0;
	cv::line(image, cv::Point2d(X1,Y1), cv::Point2d(X2,Y2), getComplexityColor(_complexity), 5);

#ifdef SAVE_DEBUG_IMAGES
	cv::line(m_debugDrawImage, cv::Point2d(X1,Y1), cv::Point2d(X2,Y2), CV_RGB(0, std::lround(255. / (cnt+1)), 0), 3);

	_saveDebugImage(image, "found_parking_" + std::to_string(cnt++), "");
#endif

	return image;
}

cv::Mat Parking::_createParkingPlaceImage(
	unsigned int _uTopShift,
	unsigned int _uWidth,
	ParkingPlace::PARK_COMPLEXITY _complexity,
	const cv::Rect & _middleBoundaries
) const {
	cv::Mat image;
	_getImageToDetectOn().copyTo(image);

	cv::rectangle(
		image,
		cv::Rect(_middleBoundaries.x, static_cast<int>(_uTopShift), _middleBoundaries.width, static_cast<int>(_uWidth)),
		getComplexityColor(_complexity),
		2
	);

	HDLOG << "Draw ParkingPlace image rect: {" << _middleBoundaries.x << "," << _uTopShift << "," << _middleBoundaries.width << "," << _uWidth << "}";

	if (_isParkingImageHasRotation()) {
#ifdef SAVE_DEBUG_IMAGES
		_saveDebugImage(image, "freeparking_before_rotate", "y" + std::to_string(_uTopShift));
#endif

		// rotate image back
		cv::Mat rotated = rotateImage(image, -m_options.getRotationDegree());
		cv::Size rotatedSize = rotated.size();
		cv::Size originalSize = m_lastParkingImage.size();

		// crop image to original size because of possible appeared black borders after rotating to arbitrary angle
		cv::Rect cropRect(
			std::max(0, static_cast<int>(std::lround((rotatedSize.width - originalSize.width)/2.))),
			std::max(0, static_cast<int>(std::lround((rotatedSize.height - originalSize.height)/2.))),
			originalSize.width,
			originalSize.height
		);

		image = rotated(cropRect);
	}

#ifdef SAVE_DEBUG_IMAGES
	_saveDebugImage(image, "freeparking", "y" + std::to_string(_uTopShift));
#endif

	return image;
}

unsigned int Parking::_getDefaultCarWidth() const {
	const int nImageHeight = static_cast<int>(_getDetectionImageHeight());

	const int nBottomBoundary =
		inRange(nImageHeight, 0 + m_options.getShiftTop(), nImageHeight - m_options.getShiftBottom());

	const int nTopBoundary = inRange(0, 0 + m_options.getShiftTop(), nImageHeight - m_options.getShiftBottom());

	const int nSpace = nBottomBoundary - nTopBoundary;
	const int nCarsCapacity = m_options.getParkingCapacity();

	// The parking of width 'nSpace' can accomodate 'nCarsCapacity' cars.
	// So, calculate one car's width as nSpace/nCarsCapacity.

	return nSpace <= 0 || nCarsCapacity <= 0
		? 0
		: static_cast<unsigned int>(std::lround(static_cast<double>(nSpace)/nCarsCapacity));
}

cv::Rect Parking::_getMiddleParkingPlacesBoundaries() const {
	int nLeft = 0;
	int nWidth = 0;

	const int nImageWidth = static_cast<int>(_getDetectionImageWidth());
	const std::vector<TFDetectedObject> & objects = m_pDetectedObjects->getDetectedObjects();

	if (!objects.empty()) {
		for (const TFDetectedObject & obj : objects) {
			nLeft += obj.getBBox().x;
			nWidth += obj.getBBox().width;
		}

		nLeft = inRange(static_cast<int>(std::lround(static_cast<double>(nLeft) / objects.size())), 0, nImageWidth);
		nWidth = inRange(static_cast<int>(std::lround(static_cast<double>(nWidth) / objects.size())), 0, nImageWidth);
	} else {
		// no objects detected, get boundaries as 20% from each edge
		nLeft = static_cast<int>(std::lround(nImageWidth*0.2));
		nWidth = static_cast<int>(std::lround(nImageWidth*0.6)); // 60% = 100% - 20% (from left edge) - 20% (from right edge)
	}

	return cv::Rect(nLeft, 0, nWidth, static_cast<int>(_getDetectionImageHeight()));
}

void Parking::_saveDetectionImages() {
	cv::Mat detected;
	_getImageToDetectOn().copyTo(detected);
	_drawBBoxes(detected);

#ifdef SAVE_DEBUG_IMAGES
	_drawBBoxes(m_debugDrawImage);
	_saveDebugImage(m_debugDrawImage, "debugDrawImage", "");
	_saveDebugImage(m_lastParkingImage, "input_parking", "");
	_saveDebugImage(_getImageToDetectOn(), "input_detector", "");
	_saveDebugImage(detected, "detected", "");
#else
	cv::imwrite("detected_" + m_options.getParkingName() + ".png", detected);
#endif
}

void Parking::_drawBBoxes(cv::Mat & _image) const {
	for (const TFDetectedObject & obj : m_pDetectedObjects->getDetectedObjects()) {
		const cv::Rect & bbox = obj.getBBox();
		cv::rectangle(_image, bbox, CV_RGB(255, 0, 0));

		std::stringbuf sbuf;
		std::ostream os(&sbuf);
		os << std::fixed << std::setprecision(2) << obj.getScore()*100 << "% id" << obj.getId();

		if (!_isObjectIntersectsWithParkingArea(obj))
			os << " (skip)";

		cv::putText(
			_image,
			sbuf.str().c_str(),
			cv::Point(bbox.x+2, bbox.y+12),
			CV_FONT_HERSHEY_PLAIN,
			1.,
			CV_RGB(255, 0, 0)
		);

		// draw hull
		if (!obj.getConvexHull().empty()) {
			std::vector<std::vector<cv::Point>> hulls;
			hulls.push_back(obj.getConvexHull());
			cv::drawContours(_image, hulls, 0, CV_RGB(0, 255, 0));
		}
	}
}

void Parking::_drawLinkingLineWithText(
	cv::Mat & _image,
	double _distance,
	const cv::Scalar & _color,
	int thickness,
	const std::string & _text,
	bool _bIsTextNearFarLine
) const {
	// constants
	const double WcloseLine = m_options.getParkingLineLengthInPixels();
	const double KcloseLine = m_options.getParkingLineDistortionFactor();

	const double WfarLine = m_options.getParkingFarLineLengthInPixels();
	const double KfarLine = m_options.getParkingFarLineDistortionFactor();

	const double closeLineAngle = m_options.getParkingLineAngle();
	const double farLineAngle = m_options.getParkingFarLineAngle();

	const double L = m_options.getParkingLengthInMeters();

	const cv::Point & parkingLineStartPoint = m_options.getParkingLineStartPoint();
	const cv::Point & parkingFarLineStartPoint = m_options.getParkingFarLineStartPoint();

	// calculate coordinates
	const double closeLinePixels = calcDistancePixels(_distance, WcloseLine, L, KcloseLine);
	const double farLinePixels = calcDistancePixels(_distance, WfarLine, L, KfarLine);

	const double Xclose = parkingLineStartPoint.x + closeLinePixels * std::cos(closeLineAngle);
	const double Xfar = parkingFarLineStartPoint.x + farLinePixels * std::cos(farLineAngle);

	const double Yclose = parkingLineStartPoint.y + closeLinePixels * std::sin(closeLineAngle);
	const double Yfar = parkingFarLineStartPoint.y + farLinePixels * std::sin(farLineAngle);

	// do drawing
	cv::line(_image, cv::Point2d(Xclose,Yclose), cv::Point2d(Xfar,Yfar), _color, thickness);

	if (!_text.empty()) {
		cv::putText(
			_image,
			_text.c_str(),
			_bIsTextNearFarLine ? cv::Point2d(Xfar - 10, Yfar - 5) : cv::Point2d(Xclose, Yclose + 13.),
			CV_FONT_HERSHEY_PLAIN,
			1.,
			_color
		);
	}
}

void Parking::_drawLinkingLine(cv::Mat & _image, double _distance, const cv::Scalar & _color, int _thickness) const {
	_drawLinkingLineWithText(_image, _distance, _color, _thickness, "", false);
}

void Parking::_drawParkingEdges(cv::Mat & _image) const {
	// draw lines in five points: [0, 1/4, 1/2, 3/4, 1] of the parking length
	const double dLength = m_options.getParkingLengthInMeters();
	std::vector<double> drawPoints(5);
	drawPoints[0] = 0.;
	drawPoints[1] = dLength / 4.;
	drawPoints[2] = dLength / 2.;
	drawPoints[3] = 3. * dLength / 4.;
	drawPoints[4] = dLength;

	for (size_t i = 0; i < drawPoints.size(); ++i)
		_drawLinkingLine(_image, drawPoints[i], DEBUG_IMAGE_PARKING_EDGES_COLOR, 1);

	cv::line(_image, m_options.getParkingLineStartPoint(), m_options.getParkingLineEndPoint(), DEBUG_IMAGE_PARKING_EDGES_COLOR);
	cv::line(_image, m_options.getParkingFarLineStartPoint(), m_options.getParkingFarLineEndPoint(), DEBUG_IMAGE_PARKING_EDGES_COLOR);
}

#ifdef SAVE_DEBUG_IMAGES
void Parking::_saveDebugImage(
	const cv::Mat & _image,
	const std::string & _strPrefix,
	const std::string & _strPostfix
) const {
	const std::string strName =
		(_strPrefix.empty() ? "" : _strPrefix + "_") +
		m_options.getParkingName() + "_" + std::to_string(m_luLastImageId) +
		(_strPostfix.empty() ? "" : "_" + _strPostfix) +
		".png";
	cv::imwrite(strName, _image);
}
#endif // SAVE_DEBUG_IMAGES

bool Parking::_isParkingImageHasRotation() const {
	return std::abs(m_options.getRotationDegree()) > MathUtils::EPSILON;
}

void Parking::_initImageToDetectOn() {
	if (m_pParkingImageToDetectOn)
		return;  // already initialized

	if (!_isParkingImageHasRotation() && !m_options.isUseCameraDistanceMethod())
		return;  // nothing special to do with input image

	m_pParkingImageToDetectOn = std::make_unique<cv::Mat>(_createImageToDetectOn());
}

cv::Mat Parking::_createImageToDetectOn() const {
	if (m_options.isUseCameraDistanceMethod())
		return clipByContour(m_lastParkingImage, m_options.getParkingImageContour());

	return rotateImage(m_lastParkingImage, m_options.getRotationDegree());
}

const cv::Mat & Parking::_getImageToDetectOn() const {
	return m_pParkingImageToDetectOn ? *m_pParkingImageToDetectOn : m_lastParkingImage;
}
