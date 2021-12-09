#ifndef CarDetectorException_H
#define CarDetectorException_H

#include "TFException.h"

class BadCameraURIException : public TFException
{
public:
	BadCameraURIException(const std::string & _strMessage = "") : TFException(_strMessage) {}
};

class ParseOptionsException : public TFException
{
public:
	ParseOptionsException(const std::string & _strMessage = "") : TFException(_strMessage) {}
};

#endif // CarDetectorException_H
