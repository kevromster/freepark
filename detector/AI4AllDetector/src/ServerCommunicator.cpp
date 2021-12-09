#include "ServerCommunicator.h"
#include "AI4AllDetectorLog.h"

#include <nlohmann/json.hpp>

#include <boost/asio.hpp>

namespace {

const std::string AI4ALL_SERVER = "18.196.226.205";
const std::string AI4ALL_SERVER_PORT = "80";

const std::string INVALID_RESPONSE = "Invalid response";

std::vector<SubmitCameraItem> parseJson(const nlohmann::json & json) {
	std::vector<SubmitCameraItem> result;
	result.reserve(json.size());

	for (const auto & element : json) {
		const int64_t id = element.at("id");
		const int64_t tgChatId = element.at("tg_chat_id");
		const std::string url = element.at("url");
		const std::string whatToDetect = element.at("what_to_detect");
		const std::string cameraName = element.at("name");
		const int detectionThreshold = element.at("detection_threshold");
		const bool wasObjectPresentedLastTime = element.at("last_time_object_presented");
		const int edgeLeft = element.at("edge_left");
		const int edgeTop = element.at("edge_top");
		const int edgeRight = element.at("edge_right");
		const int edgeBottom = element.at("edge_bottom");
		const NotificationType notificationType = string2notificationType(element.at("notification_type"));

		result.emplace_back(
			id, tgChatId, cameraName, url, whatToDetect, detectionThreshold, notificationType, wasObjectPresentedLastTime,
			edgeLeft, edgeTop, edgeRight, edgeBottom);
	}

	return result;
}

// Sends request and returns pair <status_code, response string>.
std::pair<unsigned int, std::string> sendRequest(const std::string & _requestMethod, const std::string & _url, const std::string & _jsonData) {
	HDLOG << "Send request '" << _requestMethod << " " << _url << "'...";
	using boost::asio::ip::tcp;
	boost::asio::io_service io_service;

	// Get a list of endpoints corresponding to the server name.
	tcp::resolver resolver(io_service);
	tcp::resolver::query query(AI4ALL_SERVER, AI4ALL_SERVER_PORT);
	tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

	// Try each endpoint until we successfully establish a connection.
	tcp::socket socket(io_service);
	boost::asio::connect(socket, endpoint_iterator);

	// Form the request. We specify the "Connection: close" header so that the
	// server will close the socket after transmitting the response. This will
	// allow us to treat all data up until the EOF as the content.
	boost::asio::streambuf request;
	std::ostream request_stream(&request);
	request_stream << _requestMethod << " " << _url << " HTTP/1.1\r\n"
		//<< "GET " << "/api/submit-camera-items/?detectionEnabled=true" << " HTTP/1.1\r\n"
		<< "Host: " << AI4ALL_SERVER << "\r\n"
		<< "User-Agent: C/1.0\r\n";

	if (!_jsonData.empty())	{
		request_stream
			<< "Content-Type: application/json\r\n"
			<< "Content-Length: " << _jsonData.length() << "\r\n";
	}

	request_stream
		<< "Accept: */*\r\n"
		<< "Connection: close\r\n\r\n";

	if (!_jsonData.empty()) {
		request_stream << _jsonData;
	}

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
		LOG(ERROR) << "Invalid response: status_code " << status_code << ", status_message " << status_message;
		return std::make_pair<>(status_code, INVALID_RESPONSE);
	}
	HDLOG << "Response returned with status code " << status_code;

	// Read the response headers, which are terminated by a blank line.
	boost::asio::read_until(socket, response, "\r\n\r\n");

	// Process the response headers.
	std::string header;
	while (std::getline(response_stream, header) && header != "\r");

	std::stringstream ss;

	// Write whatever content we already have to output.
	if (response.size() > 0)
		ss << &response;

	// Read until EOF, writing data to output as we go.
	boost::system::error_code error;
	while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
		ss << &response;

	if (error != boost::asio::error::eof)
		throw boost::system::system_error(error);

	if (status_code != 200) {
		LOG(ERROR) << "Bad resonse code: " << status_code << "; status_message: " << status_message;
	}

	return std::make_pair<>(status_code, ss.str());
}

} // anonymous namespace

namespace ServerCommunicator
{

std::vector<SubmitCameraItem> requestSubmitCameraItems() {
	const std::pair<unsigned int, std::string> response = sendRequest("GET", "/api/submit-camera-items/?detectionEnabled=true", "");

	if (response.first != 200 || response.second == INVALID_RESPONSE)
	{
		// some error occurred
		return std::vector<SubmitCameraItem>();
	}

	return parseJson(nlohmann::json::parse(response.second));
}

void setSubmitCameraItemObjectFound(int64_t _id, bool _objectFound) {
	HDLOG << "Sending 'ObjectFound? " << (_objectFound ? "true" : "false") << "' to the server...";

	const nlohmann::json json = {
	  {"last_time_object_presented", _objectFound}
	};
	const std::string jsonStr = json.dump();
	const std::string url = "/api/submit-camera-items/" + std::to_string(_id) + "/";

	sendRequest("PATCH", url, jsonStr);
}

} // namespace ServerCommunicator
