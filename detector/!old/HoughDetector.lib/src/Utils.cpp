#include "Utils.h"

#include <cctype>
#include <locale>
#include <sstream>
#include <algorithm>
#include <functional>

struct MatchPathSeparator
{
	bool operator()(char _ch) const {
		return _ch == '\\' || _ch == '/';
	}
};

std::string getFileName(const std::string & _strPath) {
	return std::string(
		std::find_if(_strPath.rbegin(), _strPath.rend(), MatchPathSeparator()).base(),
		_strPath.end()
	);
}

std::string removeExtension(const std::string & _strFileName) {
	std::string::const_reverse_iterator pivot =std::find(_strFileName.rbegin(), _strFileName.rend(), '.');
	return pivot == _strFileName.rend()? _strFileName : std::string(_strFileName.begin(), pivot.base() - 1);
}

std::string getExtension(const std::string & _strFileName) {
	std::string::const_reverse_iterator pivot = std::find(_strFileName.rbegin(), _strFileName.rend(), '.');
	return pivot != _strFileName.rend() ? std::string(pivot.base(), _strFileName.end()) : "";
}

std::string basename(const std::string & _strPath) {
	return removeExtension(getFileName(_strPath));
}

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
