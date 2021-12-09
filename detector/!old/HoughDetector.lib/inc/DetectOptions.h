#ifndef DetectOptions_H
#define DetectOptions_H

#include "HoughForestOptions.h"

#include <vector>

/**
 * Options for detecting objects via HoughForest algorithm.
 */
class DetectOptions : public HoughForestOptions
{
public:
	static DetectOptions createFromFile(const std::string & _strConfigFile);

	DetectOptions() :
		m_uDetectedScaling(0),
		m_uDetectionThreshold(0)
	{}

	bool areCorrect() const {
		return
			HoughForestOptions::areCorrect() &&
			!m_scales.empty() &&
			m_uDetectedScaling > 0 &&
			m_uDetectionThreshold > 0;
	}

	const std::vector<double> & getScales() const {return m_scales;}
	unsigned int getDetectedScaling() const {return m_uDetectedScaling;}
	unsigned int getDetectionThreshold() const {return m_uDetectionThreshold;}

	void addScale(double _dScale) {m_scales.push_back(_dScale);}
	void setDetectedScaling(unsigned int _uValue) {m_uDetectedScaling = _uValue;}
	void setDetectionThreshold(unsigned int _uValue) {m_uDetectionThreshold = _uValue;}

private:
	std::vector<double> m_scales;

	unsigned int m_uDetectedScaling;
	unsigned int m_uDetectionThreshold;  // threshold to cut too weak detections
};

#endif // DetectOptions_H
