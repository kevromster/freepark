#ifndef TFApp_H
#define TFApp_H

#include "ITFDetector.h"

class TFApp
{
public:
	TFApp(const std::string & _strGraphFile);

	void detect(const std::string & _strImageFile, const std::string & _strOutFile);

private:
	ITFDetector & _getDetector();

	void _drawBBoxes(
		const std::string & _strImageFile,
		const std::string & _strOutFile,
		const TFDetectedObjects & _objects
	);

private:
	std::string m_strGraphFile;
	std::unique_ptr<ITFDetector> m_pDetector;
};

#endif // TFApp_H
