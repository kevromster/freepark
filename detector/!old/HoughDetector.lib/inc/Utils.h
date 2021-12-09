#ifndef Utils_H
#define Utils_H

#include <string>
#include <vector>

// Returns filename without path: '/a/b/file.txt' -> 'file.txt'.
std::string getFileName(const std::string & _strPath);

// Returns filename without extension: 'file.txt' -> 'file'.
std::string removeExtension(const std::string & _strFileName);

// Returns file extensions: 'file.txt' -> 'txt'.
std::string getExtension(const std::string & _strFileName);

// Returns filename without path and extensions: '/a/b/file.txt' -> 'file'.
std::string basename(const std::string & _strPath);

// Trims string from start.
std::string & ltrim(std::string & _s);

// Trims string from end.
std::string & rtrim(std::string & _s);

// Trims string from both end and start.
std::string & trim(std::string & _s);

// Splits string by delimiters.
std::vector<std::string> split(const std::string & _s, char _chDelimiter);

// Appends one vector to another by moving values.
template<typename T>
void appendVector(std::vector<T> & _appendTo, std::vector<T> & _appendWhat) {
	_appendTo.reserve(_appendTo.size() + _appendWhat.size());
	_appendTo.insert(_appendTo.end(),
		std::make_move_iterator(_appendWhat.begin()),
		std::make_move_iterator(_appendWhat.end())
	);
	_appendWhat.clear();
}

#endif // Utils_H
