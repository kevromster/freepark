#ifndef TFException_H
#define TFException_H

#include "TFExport.h"

#include <string>

class TF_API TFException
{
public:
	TFException(const std::string & _strMessage = "") : m_strMessage(_strMessage) {}

	const std::string & message() const {return m_strMessage;}

private:
	std::string m_strMessage;
};

class TF_API TFImageReadException : public TFException
{
public:
	TFImageReadException(const std::string & _strFileName) : TFException("Error loading image \"" + _strFileName + "\"") {}
};

class TF_API TFLoadGraphException : public TFException
{
public:
	TFLoadGraphException(const std::string & _strMessage = "") :
		TFException(_strMessage.empty() ? "Error loading TensorFlow Graph object" : _strMessage)
	{}
};

class TF_API TFDetectionException : public TFException
{
public:
	TFDetectionException(const std::string & _strMessage) : TFException(_strMessage) {}
};

#endif // TFException_H
