#ifndef CameraOptions_H
#define CameraOptions_H

#include <map>
#include <string>
#include <vector>

class ParkingOptions;

class CameraOptions
{
public:
	static std::vector<CameraOptions> loadMultiConfig(const std::string & _strMultiConfigFile);

public:
	CameraOptions(const std::string & _strConfigFile);

	unsigned int getCameraProbeInterval() const {return m_uCameraProbeIntervalSecs;}
	long getKnownClassId(const std::string & _strLabel) const {return m_objectClassIds.at(_strLabel);}

	const std::string & getTensorFlowGraphFile() const {return m_strTensorFlowGraphFile;}
	const std::string & getCameraURI() const {return m_strCameraURI;}
	const std::string & getAuthToken() const {return m_strAuthToken;}
	const std::vector<ParkingOptions> & getParkingOptions() const {return m_parkingOptions;}

	void setCameraProbeInterval(unsigned int _uInterval) {m_uCameraProbeIntervalSecs = _uInterval;}
	void setTensorFlowGraphFile(const std::string & _strFile) {m_strTensorFlowGraphFile = _strFile;}
	void setCameraURI(const std::string & _strURI) {m_strCameraURI = _strURI;}
	void setAuthToken(const std::string & _strToken) {m_strAuthToken = _strToken;}

	void addParkingOptions(ParkingOptions && _options) {m_parkingOptions.push_back(_options);}
	void setKnownObjectClassId(const std::string & _strLabel, long _lClassId) {m_objectClassIds[_strLabel] = _lClassId;}

private:
	bool _areCorrect() const;

private:
	unsigned int m_uCameraProbeIntervalSecs;
	std::string m_strTensorFlowGraphFile;
	std::string m_strCameraURI;
	std::string m_strAuthToken;

	std::vector<ParkingOptions> m_parkingOptions;
	std::map<std::string, long> m_objectClassIds;
};

#endif // CameraOptions_H
