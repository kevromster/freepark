#ifndef DetectOptionsLoader_H
#define DetectOptionsLoader_H

#include "HoughForestOptionsLoader.h"

class DetectOptions;

class DetectOptionsLoader : public HoughForestOptionsLoader
{
public:
	DetectOptionsLoader(const std::string & _strConfigFileName, DetectOptions & _options);

protected:
	virtual void _initKnownOptions();
	virtual void _setOption(const std::string & _strOption, const std::string & _strValue);
	virtual void _checkAllOptionsSet() const;
	virtual bool _isKnownOption(const std::string & _strOption) const;

private:
	void _setScales(const std::string & _strOption);
	void _setDetectedScaling(const std::string & _strOption);
	void _setDetectionThreshold(const std::string & _strOption);

private:
	DetectOptions & m_options;
	HoughForestOptionsSetter<DetectOptionsLoader> m_optionsSetter;
};

#endif // DetectOptionsLoader_H
