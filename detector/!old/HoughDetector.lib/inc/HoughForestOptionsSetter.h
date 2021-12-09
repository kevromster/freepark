#ifndef HoughForestOptionsSetter_H
#define HoughForestOptionsSetter_H

#include "HoughForestOptions.h"
#include "HDException.h"

#include <map>

/**
 * Common setter of options which are being loaded by the options loader.
 */
template<typename TOptionsLoader>
class HoughForestOptionsSetter
{
public:
	using SetOptionFunc = void (TOptionsLoader::*)(const std::string &);

	void addKnownOption(const std::string & _strOptionName, const SetOptionFunc & _func) {
		m_knownOptions.emplace(_strOptionName, OptionSetter(_func));
	}

	void setOption(TOptionsLoader & _loader, const std::string & _strOption, const std::string & _strValue) {
		m_knownOptions.find(_strOption)->second.setOption(_loader, _strValue);
	}

	void checkAllOptionsSet() const {
		for (const auto & option : m_knownOptions)
			if (!option.second.isSet())
				throw ParseOptionsException("Option \"" + option.first + "\" is not set! Wrong config file.");
	}

	bool isKnownOption(const std::string & _strOption) const {
		return m_knownOptions.find(_strOption) != m_knownOptions.end();
	}

private:
	class OptionSetter
	{
	public:
		OptionSetter(const SetOptionFunc & _func) : m_bIsSet(false), m_setFunc(_func) {}
		void setOption(TOptionsLoader & _loader, const std::string & _strValue) {
			(_loader.*m_setFunc)(_strValue);
			m_bIsSet = true;
		}
		bool isSet() const {return m_bIsSet;}
	private:
		bool m_bIsSet;
		SetOptionFunc m_setFunc;
	};

private:
	std::map<std::string, OptionSetter> m_knownOptions;
};

#endif // HoughForestOptionsSetter_H
