#ifndef AI4AllDetectorException_H
#define AI4AllDetectorException_H

#include "TFException.h"

class BadTFLabelsFileException : public TFException
{
public:
	BadTFLabelsFileException(const std::string & _strMessage = "") : TFException(_strMessage) {}
};

class ErrorCreatingScreenshotsDirException : public TFException
{
public:
	ErrorCreatingScreenshotsDirException(const std::string & _path) : TFException("Error creating directory for screenshots: '" + _path + "'") {}
};

class BadNotificationTypeException : public TFException
{
public:
	BadNotificationTypeException(const std::string & _str) : TFException("Unknown NotificationType: " + _str) {}
};

#endif // AI4AllDetectorException_H
