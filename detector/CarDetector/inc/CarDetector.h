#ifndef CarDetector_H
#define CarDetector_H

#include "Parking.h"

#include <ITFDetector.h>

#include <chrono>

class CameraOptions;

/**
 * Detects cars and fills database of a free parking places.
 */
class CarDetector
{
public:
	CarDetector(const CameraOptions & _cameraOptions, bool _bTestMode, const std::string & _strDetectionResultFile);

	/**
	 * Runs main detection algorithm in a cycle, sleeping for specified amount of time between runs.
	 * It connects to a specified IP camera, grabs an image and processes it.
	 */
	void run();

	const std::string & getCameraURI() const {return m_strCameraUri;}
	unsigned int getCameraProbeInterval() const {return m_uCameraProbeIntervalInSecs;}

	const std::chrono::time_point<std::chrono::high_resolution_clock> & getLastStartDetectionTime() const {
		return m_lastStartDetectionTime;
	}

private:
	bool _obtainParkingImages();
	cv::Mat _captureInputImageFromFile();
	cv::Mat _captureInputImageFromNetwork() const;

	void _saveFreeParkingPlacesInDB(long _lParkingId, const std::vector<ParkingPlace> & _freePlaces) const;
	void _saveFreeParkingPlacesInFile(long _lParkingId, const std::vector<ParkingPlace> & _freePlaces) const;

	std::string _encodeToBase64(const cv::Mat  & _image) const;
	std::string _getAsJson(const std::vector<ParkingPlace> & _freePlaces) const;
	bool _sendRequest(long _lParkingId, const std::string & _json) const;

private:
	std::vector<Parking> m_parkings;
	std::unique_ptr<ITFDetector> m_pDetector;

	const bool m_bTestMode;
	const std::string m_strDetectionResultFile;

	const long m_lCarClassId;
	const unsigned int m_uCameraProbeIntervalInSecs;

	const std::string m_strCameraUri;
	const std::string m_strAuthToken;

	// when detection was started last time
	std::chrono::time_point<std::chrono::high_resolution_clock> m_lastStartDetectionTime;

	// when input image screenshot changed last time (used only for local file based mode)
	std::chrono::system_clock::time_point m_inputImageLastChangeTime;
};

#endif // CarDetector_H
