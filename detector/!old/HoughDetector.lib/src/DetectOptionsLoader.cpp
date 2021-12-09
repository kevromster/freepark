#include "DetectOptionsLoader.h"
#include "DetectOptions.h"
#include "HDException.h"
#include "Utils.h"

#include <fstream>

DetectOptionsLoader::DetectOptionsLoader(const std::string & _strConfigFileName, DetectOptions & _options) :
	HoughForestOptionsLoader(_strConfigFileName, _options),
	m_options(_options)
{
}

void DetectOptionsLoader::_initKnownOptions() {
	HoughForestOptionsLoader::_initKnownOptions();
	m_optionsSetter.addKnownOption("detect_scales", &DetectOptionsLoader::_setScales);
	m_optionsSetter.addKnownOption("detected_scaling", &DetectOptionsLoader::_setDetectedScaling);
	m_optionsSetter.addKnownOption("detection_threshold", &DetectOptionsLoader::_setDetectionThreshold);
}

void DetectOptionsLoader::_setOption(const std::string & _strOption, const std::string & _strValue) {
	if (m_optionsSetter.isKnownOption(_strOption))
		m_optionsSetter.setOption(*this, _strOption, _strValue);
	else
		HoughForestOptionsLoader::_setOption(_strOption, _strValue);
}

bool DetectOptionsLoader::_isKnownOption(const std::string & _strOption) const {
	return m_optionsSetter.isKnownOption(_strOption) || HoughForestOptionsLoader::_isKnownOption(_strOption);
}

void DetectOptionsLoader::_checkAllOptionsSet() const {
	m_optionsSetter.checkAllOptionsSet();
	HoughForestOptionsLoader::_checkAllOptionsSet();
}

void DetectOptionsLoader::_setScales(const std::string & _strOption) {
	if (_strOption.empty())
		return;

	const std::string strAcceptableFormat = "(scale1 scale2 ... scaleN)";

	if (_strOption.front() != '(' || _strOption.back() != ')')
		throw ParseOptionsException("Error parsing scales string: expected format is \"" + strAcceptableFormat + "\", got: \"" + _strOption + "\"");

	try {
		std::string strScales = _strOption;
		strScales.pop_back();
		strScales.erase(strScales.begin());
		const std::vector<std::string> vScales = split(strScales, ' ');

		for (const auto & strScale : vScales)
			m_options.addScale(std::stod(strScale));
	} catch (const std::invalid_argument & _ex) {
		throw ParseOptionsException(std::string("Error parsing scales string: ") + _ex.what());
	}
}

void DetectOptionsLoader::_setDetectedScaling(const std::string & _strOption) {
	unsigned int uValue = 0;
	std::istringstream(_strOption) >> uValue;
	if (uValue > 0)
		m_options.setDetectedScaling(uValue);
}

void DetectOptionsLoader::_setDetectionThreshold(const std::string & _strOption) {
	unsigned int uValue = 0;
	std::istringstream(_strOption) >> uValue;
	if (uValue > 0)
		m_options.setDetectionThreshold(uValue);
}
