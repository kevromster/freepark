#include "Parkings.h"

#include <fstream>

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

std::pair<cv::Rect, double> parseRectangleAndAngle(const std::string & _strOption) {
	// parsing line like
	//   {1550, 70, 360, 130, 90.333}

	std::vector<std::string> vSplitted = split(_strOption, ',');

	if (vSplitted.size() != 5)
		throw Parkings::ParseException();

	const std::string & strX = trim(vSplitted[0]);
	const std::string & strY = trim(vSplitted[1]);
	const std::string & strW = trim(vSplitted[2]);
	const std::string & strH = trim(vSplitted[3]);
	const std::string & strA = trim(vSplitted[4]);

	if (strX.front() != '{' || strA.back() != '}')
		throw Parkings::ParseException();

	int x = std::stoi(strX.substr(1));
	int y = std::stoi(strY);
	int w = std::stoi(strW);
	int h = std::stoi(strH);
	double dAngle = std::stod(strA);

	return std::make_pair<>(cv::Rect(x, y, w, h), dAngle);
}


} // anonymous namespace

Parkings::Parkings(const std::string & _strConfigFile) {
	if (_strConfigFile.empty())
		throw ParseException();

	std::ifstream in(_strConfigFile);

	if (!in.is_open())
		throw ParseException();

	std::string line;
	std::vector<std::string> vSplittedLine;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		std::getline(in, line);
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		vSplittedLine = split(line, '=');

		if (vSplittedLine.size() != 2)
			throw ParseException();

		auto rectAndAngle = parseRectangleAndAngle(trim(vSplittedLine[1]));
		m_parkings.emplace_back(trim(vSplittedLine[0]), rectAndAngle.first, rectAndAngle.second);
	}
}
