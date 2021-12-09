#include "CameraOptions.h"
#include "CarDetectorLog.h"
#include "CarDetectorException.h"
#include "ParkingOptions.h"

#include <fstream>
#include <unordered_set>

#include <boost/algorithm/string.hpp>

namespace {

std::string & ltrim(std::string & _s) {
	_s.erase(_s.begin(), std::find_if(_s.begin(), _s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return _s;
}

std::string & rtrim(std::string & _s) {
	_s.erase(std::find_if(_s.rbegin(), _s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), _s.end());
	return _s;
}

std::string & trim(std::string & _s) {
	return ltrim(rtrim(_s));
}

std::vector<std::string> split(const std::string & _s, char _chDelimiter) {
	std::vector<std::string> elements;
	std::stringstream ss(_s);
	std::string item;
	while (std::getline(ss, item, _chDelimiter))
		if (!item.empty())
			elements.push_back(item);
	return elements;
}

class OptionsLoader;

class CameraOptionsSetter
{
public:
	using SetOptionFunc = void (OptionsLoader::*)(const std::string &);

	void addKnownOption(const std::string & _strOptionName, const SetOptionFunc & _func) {
		// non-optional (i.e. mandatory) by default
		addKnownOption(_strOptionName, _func, false);
	}

	void addKnownOption(const std::string & _strOptionName, const SetOptionFunc & _func, bool isOptional) {
		m_knownOptions.emplace(_strOptionName, OptionSetter(_func, isOptional));
	}

	void setOption(OptionsLoader & _loader, const std::string & _strOption, const std::string & _strValue) {
		m_knownOptions.find(_strOption)->second.setOption(_loader, _strValue);
	}

	void checkAllOptionsSet() const {
		for (const auto & option : m_knownOptions)
			if (!option.second.isSet()) {
				if (!option.second.isOptional())
					throw ParseOptionsException("Option \"" + option.first + "\" is not set! Wrong config file.");
				HDLOG << "Option \"" << option.first + "\" is not set! Default will be used...";
			}
	}

	bool isKnownOption(const std::string & _strOption) const {
		return m_knownOptions.find(_strOption) != m_knownOptions.end();
	}

private:
	class OptionSetter
	{
	public:
		OptionSetter(const SetOptionFunc & _func, bool _isOptional) : m_bIsSet(false), m_bIsOptional(_isOptional), m_setFunc(_func) {}
		void setOption(OptionsLoader & _loader, const std::string & _strValue) {
			(_loader.*m_setFunc)(_strValue);
			m_bIsSet = true;
		}
		bool isSet() const {return m_bIsSet;}
		bool isOptional() const {return m_bIsOptional;}
	private:
		bool m_bIsSet;
		bool m_bIsOptional;  // if optional, the option can absent in a config file
		SetOptionFunc m_setFunc;
	};

private:
	std::map<std::string, OptionSetter> m_knownOptions;
};

class OptionsLoader
{
public:
	OptionsLoader(const std::string & _strConfigFileName, CameraOptions & _options) :
		m_options(_options),
		m_strConfigFileName(_strConfigFileName),
		m_bParseParkingSection(false)
	{}

	void initKnownOptions();
	void load();
	void checkAllOptionsSet() const;

private:
	void _setOption(const std::string & _strOption, const std::string & _strValue);
	bool _isKnownOption(const std::string & _strOption) const;

	GeoCoordinatePoint _parseGeoCoordinate(const std::string & _strOption, const std::string & _strOptionName) const;
	cv::Rect _parseRectangle(const std::string & _strOption, const std::string & _strOptionName) const;
	cv::Point _parsePoint(const std::string & _strOption, const std::string & _strOptionName) const;
	std::vector<cv::Point> _parsePoints(const std::string & _strOption, const std::string & _strOptionName, size_t _nMinimalExpectedCount) const;

private:
	void _setTensorFlowGraphFile(const std::string & _strOption);
	void _setCameraURI(const std::string & _strOption);
	void _setCameraProbeInterval(const std::string & _strOption);
	void _setCameraAuthToken(const std::string & _strOption);
	void _setParkingId(const std::string & _strOption);
	void _setParkingCapacity(const std::string & _strOption);
	void _setParkingName(const std::string & _strOption);
	void _setParkingRectangle(const std::string & _strOption);
	void _setParkingContour(const std::string & _strOption);
	void _setParkingLineDelta(const std::string & _strOption);
	void _setParkingLine(const std::string & _strOption);
	void _setParkingFarLine(const std::string & _strOption);
	void _setParkingStartPoint(const std::string & _strOption);
	void _setParkingEndPoint(const std::string & _strOption);
	void _setParkingRotationDegree(const std::string & _strOption);
	void _setShiftTop(const std::string & _strOption);
	void _setShiftBottom(const std::string & _strOption);
	void _setParkingDetectionThreshold(const std::string & _strOption);
	void _setUseCameraDistanceMethod(const std::string & _strOption);

	void _startParkingParsing(const std::string & _strOption);
	void _endParkingParsing();

	void _parseTensorFlowLabelsFile(const std::string & _strLabelsFile);

private:
	CameraOptions & m_options;
	std::string m_strConfigFileName;
	CameraOptionsSetter m_optionsSetter;

	bool m_bParseParkingSection;
	ParkingOptions m_parseParkingOptions;
};

} // anonymous namespace

std::vector<CameraOptions> CameraOptions::loadMultiConfig(const std::string & _strMultiConfigFile) {
	if (_strMultiConfigFile.empty())
		throw ParseOptionsException("bad multi-config file name");

	std::ifstream in(_strMultiConfigFile);

	if (!in.is_open())
		throw ParseOptionsException("Multi-config file not found: " + _strMultiConfigFile);

	std::vector<CameraOptions> cameras;
	std::string line;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		std::getline(in, line);
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		cameras.emplace_back(line);
	}

	return cameras;
}

CameraOptions::CameraOptions(const std::string & _strConfigFile) :
	m_uCameraProbeIntervalSecs(0)
{
	OptionsLoader loader(_strConfigFile, *this);
	loader.initKnownOptions();
	loader.load();
	loader.checkAllOptionsSet();

	if (!_areCorrect())
		throw ParseOptionsException("Some options are INCORRECT after parsing!");
}

bool CameraOptions::_areCorrect() const {
	if (m_uCameraProbeIntervalSecs == 0 ||
		m_strTensorFlowGraphFile.empty() ||
		m_strCameraURI.empty() ||
		m_strAuthToken.empty() ||
		m_parkingOptions.empty() ||
		m_objectClassIds.empty()
	)
		return false;

	std::unordered_set<long> existingIds;

	for (const ParkingOptions & opts : m_parkingOptions) {
		if (opts.getParkingName().empty() ||
			opts.getParkingImageRectangle().area() == 0 ||
			opts.getDetectionThreshold() <= 0. ||
			opts.getDetectionThreshold() > 1. ||
			opts.getParkingLengthInMeters() <= 0. ||
			(opts.isUseCameraDistanceMethod() &&
			 (opts.getParkingImageContour().size() < 3 ||
			  opts.getParkingLineLengthInPixels() <= 0. ||
			  opts.getParkingFarLineLengthInPixels() <= 0. ||
			  opts.getParkingLineDistortionFactor() <= 0. ||
			  opts.getParkingFarLineDistortionFactor() <= 0. ||
			  opts.getParkingEdges().size() != 4 ||
			  opts.getParkingEdgesBoundingRectangle().area() == 0)) ||
			existingIds.count(opts.getParkingId()) > 0
		)
			return false;
		existingIds.insert(opts.getParkingId());
	}

	return true;
}

void OptionsLoader::initKnownOptions() {
	m_optionsSetter.addKnownOption("tensorflow_graph_file", &OptionsLoader::_setTensorFlowGraphFile);
	m_optionsSetter.addKnownOption("tensorflow_labels_file", &OptionsLoader::_parseTensorFlowLabelsFile);
	m_optionsSetter.addKnownOption("camera_uri", &OptionsLoader::_setCameraURI);
	m_optionsSetter.addKnownOption("camera_probe_interval", &OptionsLoader::_setCameraProbeInterval);
	m_optionsSetter.addKnownOption("camera_token", &OptionsLoader::_setCameraAuthToken);
	m_optionsSetter.addKnownOption("parking", &OptionsLoader::_startParkingParsing);
	m_optionsSetter.addKnownOption("parking_id", &OptionsLoader::_setParkingId);
	m_optionsSetter.addKnownOption("parking_name", &OptionsLoader::_setParkingName);
	m_optionsSetter.addKnownOption("parking_capacity", &OptionsLoader::_setParkingCapacity);
	m_optionsSetter.addKnownOption("parking_rectangle", &OptionsLoader::_setParkingRectangle);
	m_optionsSetter.addKnownOption("parking_contour", &OptionsLoader::_setParkingContour, true);
	m_optionsSetter.addKnownOption("parking_line_delta", &OptionsLoader::_setParkingLineDelta, true);
	m_optionsSetter.addKnownOption("parking_line", &OptionsLoader::_setParkingLine, true);
	m_optionsSetter.addKnownOption("parking_far_line", &OptionsLoader::_setParkingFarLine, true);
	m_optionsSetter.addKnownOption("parking_start_point", &OptionsLoader::_setParkingStartPoint);
	m_optionsSetter.addKnownOption("parking_end_point", &OptionsLoader::_setParkingEndPoint);
	m_optionsSetter.addKnownOption("parking_rotation_degree", &OptionsLoader::_setParkingRotationDegree);
	m_optionsSetter.addKnownOption("parking_place_shift_top", &OptionsLoader::_setShiftTop);
	m_optionsSetter.addKnownOption("parking_place_shift_bottom", &OptionsLoader::_setShiftBottom);
	m_optionsSetter.addKnownOption("parking_detection_threshold", &OptionsLoader::_setParkingDetectionThreshold);
	m_optionsSetter.addKnownOption("parking_use_camera_distance_method", &OptionsLoader::_setUseCameraDistanceMethod, true);
}

void OptionsLoader::checkAllOptionsSet() const {
	m_optionsSetter.checkAllOptionsSet();
}

bool OptionsLoader::_isKnownOption(const std::string & _strOption) const {
	return m_optionsSetter.isKnownOption(_strOption);
}

void OptionsLoader::_setOption(const std::string & _strOption, const std::string & _strValue) {
	m_optionsSetter.setOption(*this, _strOption, _strValue);
}

void OptionsLoader::load() {
	if (m_strConfigFileName.empty())
		throw ParseOptionsException("bad config file name");

	std::ifstream in(m_strConfigFileName);

	if (!in.is_open())
		throw ParseOptionsException("Main config file not found: " + m_strConfigFileName);

	std::string line;
	std::vector<std::string> vSplittedLine;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		std::getline(in, line);
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		if (line == "}") {
			_endParkingParsing();
			continue;
		}

		const size_t eqPos = line.find_first_of("=");
		if (eqPos == std::string::npos) {
			throw ParseOptionsException(
				std::string("Wrong option description at line ") + std::to_string(uLine) +
				" (acceptable format: <option>=<value>)");
		}

		std::string strOption = line.substr(0, eqPos);
		std::string strValue = line.substr(eqPos + 1);
		trim(strOption);
		trim(strValue);

		if (!_isKnownOption(strOption)) {
			throw ParseOptionsException(
				std::string("Unknown option \"") + strOption + "\" at line " + std::to_string(uLine));
		}

		// set value
		HDLOG << "Setting option \"" << strOption << "\" to \"" << strValue << "\"...";
		_setOption(strOption, strValue);
	}
}

void OptionsLoader::_setTensorFlowGraphFile(const std::string & _strOption) {
	m_options.setTensorFlowGraphFile(_strOption);
}

void OptionsLoader::_setCameraURI(const std::string & _strOption) {
	m_options.setCameraURI(_strOption);
}

void OptionsLoader::_setCameraProbeInterval(const std::string & _strOption) {
	int nValue = std::stoi(_strOption);
	if (nValue > 0)
		m_options.setCameraProbeInterval(static_cast<unsigned int>(nValue));
}

void OptionsLoader::_setCameraAuthToken(const std::string & _strOption) {
	m_options.setAuthToken(_strOption);
}

void OptionsLoader::_startParkingParsing(const std::string & _strOption) {
	if (_strOption != "{")
		throw ParseOptionsException("Bad 'parking' option: expected 'parking = {'");

	if (m_bParseParkingSection)
		throw ParseOptionsException("Error entering in parking parse mode: already in that mode!");

	m_parseParkingOptions = ParkingOptions();
	m_bParseParkingSection = true;
}

void OptionsLoader::_endParkingParsing() {
	if (!m_bParseParkingSection)
		throw ParseOptionsException("Error exiting from parking parse mode: NOT in that mode!");

	m_options.addParkingOptions(std::move(m_parseParkingOptions));
	m_bParseParkingSection = false;
}

void OptionsLoader::_setParkingId(const std::string & _strOption) {
	m_parseParkingOptions.setParkingId(std::stol(_strOption));
}

void OptionsLoader::_setParkingCapacity(const std::string & _strOption) {
	m_parseParkingOptions.setParkingCapacity(std::stoi(_strOption));
}

void OptionsLoader::_setParkingName(const std::string & _strOption) {
	m_parseParkingOptions.setParkingName(_strOption);
}

void OptionsLoader::_setUseCameraDistanceMethod(const std::string & _strOption) {
	const std::string strLowerCaseOption = boost::algorithm::to_lower_copy(_strOption);

	if (strLowerCaseOption != "true" && strLowerCaseOption != "false")
		throw ParseOptionsException("Error parsing option 'parking_use_camera_distance_method': " + _strOption);

	m_parseParkingOptions.setUseCameraDistanceMethod(strLowerCaseOption == "true");
}

void OptionsLoader::_setParkingRectangle(const std::string & _strOption) {
	m_parseParkingOptions.setParkingImageRectangle(_parseRectangle(_strOption, "parking_rectangle"));
}

void OptionsLoader::_setParkingContour(const std::string & _strOption) {
	m_parseParkingOptions.setParkingImageContour(_parsePoints(_strOption, "parking_contour", 3));
}

void OptionsLoader::_setParkingLineDelta(const std::string & _strOption) {
	m_parseParkingOptions.setParkingLineDelta(std::stoi(_strOption));
}

void OptionsLoader::_setParkingLine(const std::string & _strOption) {
	const std::vector<cv::Point> points = _parsePoints(_strOption, "parking_line", 3);

	if (points.size() != 3)
		throw ParseOptionsException("Error parsing option 'parking_line', three points must be given: " + _strOption);

	m_parseParkingOptions.setParkingLine(points[0], points[1], points[2]);
}

void OptionsLoader::_setParkingFarLine(const std::string & _strOption) {
	const std::vector<cv::Point> points = _parsePoints(_strOption, "parking_far_line", 3);

	if (points.size() != 3)
		throw ParseOptionsException("Error parsing option 'parking_far_line', three points must be given: " + _strOption);

	m_parseParkingOptions.setParkingFarLine(points[0], points[1], points[2]);
}

void OptionsLoader::_setParkingStartPoint(const std::string & _strOption) {
	m_parseParkingOptions.setStartPoint(_parseGeoCoordinate(_strOption, "parking_start_point"));
}

void OptionsLoader::_setParkingEndPoint(const std::string & _strOption) {
	m_parseParkingOptions.setEndPoint(_parseGeoCoordinate(_strOption, "parking_end_point"));
}

GeoCoordinatePoint OptionsLoader::_parseGeoCoordinate(
	const std::string & _strOption,
	const std::string & _strOptionName
) const {
	// parking string like
	//   {54.853321, 83.040437}

	std::vector<std::string> vSplitted = split(_strOption, ',');

	if (vSplitted.size() != 2)
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	const std::string & strLat = trim(vSplitted[0]);
	const std::string & strLon = trim(vSplitted[1]);

	if (strLat.front() != '{' || strLon.back() != '}')
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	return GeoCoordinatePoint(std::stod(strLat.substr(1)), std::stod(strLon));
}

cv::Rect OptionsLoader::_parseRectangle(const std::string & _strOption, const std::string & _strOptionName) const {
	// parsing string like
	//   {1550, 70, 360, 130}

	std::vector<std::string> vSplitted = split(_strOption, ',');

	if (vSplitted.size() != 4)
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	const std::string & strX = trim(vSplitted[0]);
	const std::string & strY = trim(vSplitted[1]);
	const std::string & strW = trim(vSplitted[2]);
	const std::string & strH = trim(vSplitted[3]);

	if (strX.front() != '{' || strH.back() != '}')
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	int x = std::stoi(strX.substr(1));
	int y = std::stoi(strY);
	int w = std::stoi(strW);
	int h = std::stoi(strH);

	return cv::Rect(x, y, w, h);
}

cv::Point OptionsLoader::_parsePoint(const std::string & _strOption, const std::string & _strOptionName) const {
	// parsing string like
	//   {1550, 70}

	std::vector<std::string> vSplitted = split(_strOption, ',');

	if (vSplitted.size() != 2)
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	const std::string & strX = trim(vSplitted[0]);
	const std::string & strY = trim(vSplitted[1]);

	if (strX.front() != '{' || strY.back() != '}')
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	int x = std::stoi(strX.substr(1));
	int y = std::stoi(strY);

	return cv::Point(x, y);
}

std::vector<cv::Point> OptionsLoader::_parsePoints(
	const std::string & _strOption,
	const std::string & _strOptionName,
	size_t _nMinimalExpectedCount
) const {
	// parking string like
	//   {{0,140}; {530,0}; {669,130}; {90, 359}}

	std::vector<std::string> vSplitted = split(_strOption, ';');

	if (vSplitted.size() < _nMinimalExpectedCount)
		throw ParseOptionsException(
			"Error parsing option '" + _strOptionName +
			"', at least " + std::to_string(_nMinimalExpectedCount) + " points must be given: " +
			_strOption
		);

	std::vector<cv::Point> points;
	points.reserve(vSplitted.size());

	// parse first point
	const std::string & strFirst = trim(vSplitted.front());
	if (strFirst.front() != '{')
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	std::string strFirstCutBracket = strFirst.substr(1);
	points.push_back(_parsePoint(trim(strFirstCutBracket), _strOptionName));

	// parse from second till last-1 points
	for (size_t i = 1; i < vSplitted.size() - 1; ++i)
		points.push_back(_parsePoint(trim(vSplitted[i]), _strOptionName));

	// parse last point
	const std::string & strLast = trim(vSplitted.back());
	if (strLast.back() != '}')
		throw ParseOptionsException("Error parsing option '" + _strOptionName + "': " + _strOption);

	std::string strLastCutBracket = strLast.substr(0, strLast.length() - 1);
	points.push_back(_parsePoint(trim(strLastCutBracket), _strOptionName));

	return points;
}

void OptionsLoader::_setParkingRotationDegree(const std::string & _strOption) {
	m_parseParkingOptions.setRotationDegree(std::stod(_strOption));
}

void OptionsLoader::_setShiftTop(const std::string & _strOption) {
	m_parseParkingOptions.setShiftTop(std::stoi(_strOption));
}

void OptionsLoader::_setShiftBottom(const std::string & _strOption) {
	m_parseParkingOptions.setShiftBottom(std::stoi(_strOption));
}

void OptionsLoader::_setParkingDetectionThreshold(const std::string & _strOption) {
	m_parseParkingOptions.setDetectionThreshold(std::stod(_strOption));
}

void OptionsLoader::_parseTensorFlowLabelsFile(const std::string & _strLabelsFile) {
	// parsing file with sections like
	//   item {
	//     name: "/m/01g317"
	//     id: 1
	//     display_name: "person"
	//   }

	if (_strLabelsFile.empty())
		throw ParseOptionsException("bad tensorflow labels file name");

	std::ifstream in(_strLabelsFile);

	if (!in.is_open())
		throw ParseOptionsException("Tensorflow labels file not found: " + _strLabelsFile);

	std::string line;
	std::vector<std::string> vSplittedLine;

	std::string strParsedLabel;
	int nParsedClassId = -1;
	bool bIsIdParsed = false;
	bool bDisplayNameFound = false;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		std::getline(in, line);
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		if (line == "}") {
			if (!bIsIdParsed || strParsedLabel.empty())
				throw ParseOptionsException("Error parsing tensorflow labels file");

			HDLOG << "Setting known object class id " << nParsedClassId << " to \"" << strParsedLabel << "\"...";
			m_options.setKnownObjectClassId(strParsedLabel, nParsedClassId);

			nParsedClassId = -1;
			strParsedLabel = "";
			bIsIdParsed = false;
			bDisplayNameFound = false;

			continue;
		}

		vSplittedLine = split(line, ' ');

		if (vSplittedLine.size() < 2) {
			throw ParseOptionsException(
				std::string("Wrong option in tensorflow labels file at line ") + std::to_string(uLine));
		}

		const std::string & strFirst = trim(vSplittedLine[0]);
		const std::string & strSecond = trim(vSplittedLine[1]);

		if (strFirst == "item") {
			if (strSecond != "{" || vSplittedLine.size() != 2) {
				throw ParseOptionsException(
					std::string("Wrong option in tensorflow labels file at line ") + std::to_string(uLine));
			}
			// new section starts
			continue;
		}

		vSplittedLine = split(line, ':');

		if (vSplittedLine.size() != 2) {
			throw ParseOptionsException(
				std::string("Wrong option in tensorflow labels file at line ") + std::to_string(uLine));
		}

		const std::string & strOption = trim(vSplittedLine[0]);
		const std::string & strValue = trim(vSplittedLine[1]);

		if (strOption == "id") {
			nParsedClassId = std::stoi(strValue);
			bIsIdParsed = true;
		} else if (strOption == "display_name" || (strOption == "name" && !bDisplayNameFound)) {
			// "display_name" is always preferred overt just "name"
			strParsedLabel = strValue;
			strParsedLabel.erase(std::remove(strParsedLabel.begin(), strParsedLabel.end(), '"'), strParsedLabel.end());
			bDisplayNameFound = strOption == "display_name";
		}
	}
}
