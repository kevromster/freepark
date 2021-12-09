#ifndef HDApp_H
#define HDApp_H

#include "IHDDetector.h"
#include "IHDTrainer.h"

#include <memory>

class IHDFactory;

class HDApp
{
public:
	HDApp(const std::string & _strConfigFile);

	void detect(const std::string & _strImageFile, const std::string & _strOutFile);
	void train();

private:
	IHDDetector & _getDetector();
	IHDTrainer & _getTrainer();

	void _drawBBoxes(const std::string & _strImageFile, const std::string & _strOutFile, const HDDetectedObjects & _objects);
	void _saveHoughImages(const std::string &_strOutFile, const HDDetectedObjects & _objects);

private:
	std::string m_strConfigFile;
	std::unique_ptr<IHDDetector> m_pDetector;
	std::unique_ptr<IHDTrainer> m_pTrainer;
};

#endif // HDApp_H
