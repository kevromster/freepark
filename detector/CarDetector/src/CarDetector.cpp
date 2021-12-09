#include "CarDetector.h"

// boost/asio.hpp has to be included BEFORE CarDetectorLog.h on Win32 platform,
// otherwise an error about double defining of WinSock.h appears.
#include <boost/asio.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include "CameraOptions.h"
#include "CarDetectorException.h"
#include "CarDetectorLog.h"

#include <base64.h>

#include <opencv2/highgui.hpp>

#include <iomanip>

namespace {

const std::string FREEPARK_SERVER = "18.196.226.205";
const std::string FREEPARK_SERVER_PORT = "8000";

// Label of Car objects from label-maps file used for training TensorFlow model.
const std::string CAR_LABEL = "car";

const std::string SCREENSHOT_FILE_PREFIX = "file://";

bool startsFrom(const std::string & _strCheck, const std::string & _strPrefix) {
	return
		_strCheck.size() >= _strPrefix.size() &&
		_strCheck.substr(0, _strPrefix.size()) == _strPrefix;
}

} // anonymous namespace

CarDetector::CarDetector(const CameraOptions & _cameraOptions, bool _bTestMode, const std::string & _strDetectionResultFile) :
	m_pDetector(ITFDetector::createDetector(_cameraOptions.getTensorFlowGraphFile())),
	m_bTestMode(_bTestMode),
	m_strDetectionResultFile(_strDetectionResultFile),
	m_lCarClassId(_cameraOptions.getKnownClassId(CAR_LABEL)),
	m_uCameraProbeIntervalInSecs(_cameraOptions.getCameraProbeInterval()),
	m_strCameraUri(_cameraOptions.getCameraURI()),
	m_strAuthToken(_cameraOptions.getAuthToken())
{
	for (const ParkingOptions & opts : _cameraOptions.getParkingOptions())
		m_parkings.emplace_back(opts);
}

void CarDetector::run() {
	m_lastStartDetectionTime = std::chrono::high_resolution_clock::now();

	if (_obtainParkingImages()) {
		for (Parking & parking : m_parkings) {
			parking.detectFreeParkingPlaces(*m_pDetector, m_lCarClassId);

			if (!m_bTestMode)
				_saveFreeParkingPlacesInDB(parking.getParkingId(), parking.getFreeParkingPlaces());
			else
				_saveFreeParkingPlacesInFile(parking.getParkingId(), parking.getFreeParkingPlaces());
		}
	}
}

cv::Mat CarDetector::_captureInputImageFromFile() {
	const std::string strFilePath = m_strCameraUri.substr(SCREENSHOT_FILE_PREFIX.size());
	const boost::filesystem::path path(strFilePath);

	if (!boost::filesystem::exists(path)) {
		LOG(ERROR) << "No input image file found: " << strFilePath;
		return cv::Mat();
	}

	const auto lastChangeTime = std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(path));

	if (!m_bTestMode) {
		if (lastChangeTime == m_inputImageLastChangeTime) {
			LOG(WARNING) << "Input image file '" << strFilePath <<"' has been already processed earlier, skip it!";
			return cv::Mat();
		}

		const long durationFromNow =
			std::chrono::duration_cast<std::chrono::seconds>(
				std::chrono::high_resolution_clock::now() - lastChangeTime
			).count();

		// check for too old images, use double-probing interval as safe bound
		if (durationFromNow > 0 && static_cast<unsigned long>(durationFromNow) > 2*m_uCameraProbeIntervalInSecs) {
			LOG(WARNING)
				<< "Input image '" << strFilePath << "' seems to be too old (created ~"
				<< std::lround(durationFromNow/60.) << " minutes ago), skip it!";
			return cv::Mat();
		}
	}

	cv::Mat image = cv::imread(strFilePath);

	if (!image.data) {
		LOG(ERROR) << "Error reading screenshot from file " + strFilePath;
		return cv::Mat();
	}

	m_inputImageLastChangeTime = lastChangeTime;  // remember modification time for next time check
	return image;
}

cv::Mat CarDetector::_captureInputImageFromNetwork() const {
	cv::VideoCapture capture(m_strCameraUri);

	if (!capture.isOpened()) {
		LOG(ERROR) << "Error opening camera " << m_strCameraUri;
		return cv::Mat();
	}

	cv::Mat cameraImage;
	if (!capture.read(cameraImage)) {
		LOG(ERROR) << "Error reading frame from the camera " << m_strCameraUri;
		return cv::Mat();
	}

	return cameraImage;
}

bool CarDetector::_obtainParkingImages() {
	HDLOG << "Obtaining screenshot...";

	cv::Mat cameraImage;

	if (startsFrom(m_strCameraUri, SCREENSHOT_FILE_PREFIX))
		cameraImage = _captureInputImageFromFile();
	else
		cameraImage = _captureInputImageFromNetwork();

	if (!cameraImage.data)
		return false;  // error reading input image

	cv::imwrite("input.png", cameraImage);

	// unique identifier of this input set
	static unsigned long uniqueInputImageId = 0;
	++uniqueInputImageId;

	for (Parking & parking : m_parkings)
		parking.setParkingImage(cameraImage(parking.getParkingImageRectangle()), uniqueInputImageId);

	return true;
}

void CarDetector::_saveFreeParkingPlacesInFile(long _lParkingId, const std::vector<ParkingPlace> & _freePlaces) const {
	assert(m_bTestMode);
	assert(!m_strDetectionResultFile.empty());

	std::ofstream file;
	file.open(m_strDetectionResultFile, std::ofstream::out | std::ofstream::app);

	file << "[" << std::endl;
	file << "parking id: " << _lParkingId << std::endl;
	file << "spots count: " << _freePlaces.size() << std::endl;

	if (_freePlaces.size() > 0)
		file << std::endl;

	for (const ParkingPlace & place : _freePlaces)
		file << place.toString() << std::endl;

	file << "]" << std::endl << std::endl;
	file.close();
}

void CarDetector::_saveFreeParkingPlacesInDB(long _lParkingId, const std::vector<ParkingPlace> & _freePlaces) const {
	// if there are no free parking places, an empty list will be sent
	_sendRequest(_lParkingId, _getAsJson(_freePlaces));
}

std::string CarDetector::_getAsJson(const std::vector<ParkingPlace> & _freePlaces) const {
	std::ostringstream buf;
	buf << std::fixed << std::setprecision(6) << "[";
	bool isFirstPlace = true;

	for (const ParkingPlace & parkingPlace : _freePlaces) {
		if (isFirstPlace)
			isFirstPlace = false;
		else
			buf << ",";

		buf << "{\"latitude\":\"" << parkingPlace.getPoint().getLatitude()
			<< "\",\"longitude\":\"" << parkingPlace.getPoint().getLongitude()
			<< "\",\"parking_complexity\":\"" << parkingPlace.getComplexity()
			<< (parkingPlace.hasImage() ? "\",\"image\":\"" + _encodeToBase64(parkingPlace.getImage()) : "")
			<< "\"}";
	}

	buf << "]";
	return buf.str();
}

std::string CarDetector::_encodeToBase64(const cv::Mat  & _image) const {
	// first convert to png, then encode to base64
	std::vector<unsigned char> pngImage;
	cv::imencode(".png", _image, pngImage);

	assert(pngImage.size() <= UINT_MAX);
	return "data:image/png;base64," + base64_encode(pngImage.data(), static_cast<unsigned int>(pngImage.size()));
}

bool CarDetector::_sendRequest(long _lParkingId, const std::string & _json) const {
	HDLOG << "Sending request data to the server, data length " << _json.length();

	using boost::asio::ip::tcp;
	boost::asio::io_service io_service;

	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(FREEPARK_SERVER, FREEPARK_SERVER_PORT);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	// Try each endpoint until we successfully establish a connection.
	tcp::socket socket(io_service);
	boost::asio::connect(socket, endpoint_iterator);

	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream
		<< "POST " << "/parkings/" << _lParkingId << "/set-free-parking-places/" << " HTTP/1.1\r\n"
		<< "Host: " << FREEPARK_SERVER << "\r\n"
		<< "User-Agent: C/1.0\r\n"
		<< "Content-Type: application/json\r\n"
		<< "Content-Length: " << _json.length() << "\r\n"
		<< "Authorization: Token " << m_strAuthToken << "\r\n"
		<< "Accept: */*\r\n"
		<< "Connection: close\r\n\r\n"
		<< _json;

	// Send the request.
	boost::asio::write(socket, request);

	// Read the response status line. The response streambuf will automatically
	// grow to accommodate the entire line. The growth may be limited by passing
	// a maximum size to the streambuf constructor.
	boost::asio::streambuf response;
	boost::asio::read_until(socket, response, "\r\n");

	// Check that response is OK.
	std::istream response_stream(&response);
	std::string http_version;
	response_stream >> http_version;
	unsigned int status_code;
	response_stream >> status_code;
	std::string status_message;
	std::getline(response_stream, status_message);

	if (!response_stream || http_version.substr(0, 5) != "HTTP/")
	{
		LOG(ERROR) << "Invalid response";
		return false;
	}
	HDLOG << "Response returned with status code " << status_code;

	// Read the response headers, which are terminated by a blank line.
	boost::asio::read_until(socket, response, "\r\n\r\n");

	// Process the response headers.
	std::string header;
	while (std::getline(response_stream, header) && header != "\r")
		std::cout << header << "\n";
	std::cout << "\n";

	// Write whatever content we already have to output.
	if (response.size() > 0)
		std::cout << &response;

	// Read until EOF, writing data to output as we go.
	boost::system::error_code error;
	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
		std::cout << &response;

	std::cout << "\n";

	if (error != boost::asio::error::eof)
		throw boost::system::system_error(error);

	HDLOG << "---------request SENT";
	return status_code == 200;
}
