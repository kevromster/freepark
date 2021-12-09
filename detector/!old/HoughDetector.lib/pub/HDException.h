#ifndef HDException_H
#define HDException_H

#include "HDExport.h"

#include <string>
#include <sstream>

class HD_API HDException
{
public:
	HDException(const std::string & _strMessage = "") : m_strMessage(_strMessage) {}

	const std::string & message() const {return m_strMessage;}

private:
	std::string m_strMessage;
};

class HD_API BadVersionException : public HDException
{
public:
	BadVersionException(unsigned int _uVersion, unsigned int _uExpectedVersion) :
		HDException(_createErrorVersionString(_uVersion, _uExpectedVersion))
	{}

private:
	static
	const std::string _createErrorVersionString(unsigned int _uVersion, unsigned int _uExpectedVersion) {
		std::ostringstream ss;
		ss << "Expected version " << _uExpectedVersion << "; got version " << _uVersion;
		return ss.str();
	}
};

class HD_API BadOptionException : public HDException
{
public:
	BadOptionException(const std::string & _strMessage = "") : HDException(_strMessage) {}
};

class HD_API ParseOptionsException : public BadOptionException
{
public:
	ParseOptionsException(const std::string & _strMessage = "") : BadOptionException(_strMessage) {}
};

class HD_API ImageReadException : public HDException
{
public:
	ImageReadException(const std::string & _strFileName) : HDException("Error loading image \"" + _strFileName + "\"") {}
};

class HD_API HDIOException : public HDException
{
public:
	HDIOException(const std::string & _strMessage = "") : HDException(_strMessage) {}
};

#endif // HDException_H
