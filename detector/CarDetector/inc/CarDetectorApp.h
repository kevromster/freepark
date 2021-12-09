#ifndef CarDetectorApp_H
#define CarDetectorApp_H

#include "CarDetector.h"

/**
 * The application instance.
 */
class CarDetectorApp
{
public:
	CarDetectorApp();

	/**
	 * Runs application with the specified arguments.
	 *
	 * @param _args command line input arguiments
	 * @return the result code
	 */
	int run(const std::vector<std::string> & _args);

private:
	void _runDetectors();
	void _runDetector(CarDetector & _detector);

	void _initializeDetectors(const std::vector<CameraOptions> & _cameraOptions);
	void _sleepBeforeNextRun(const CarDetector & _nextDetectorToRun) const;

	void _printUsage() const;
	bool _parseArgs(const std::vector<std::string> & _args);

	std::vector<CameraOptions> _readCameraOptions() const;

private:
	bool m_bMultiMode; // in multi-mode we expect config file with other config files
	bool m_bTestMode;  // in test mode we run detectors only once, not in cycle

	std::string m_strDetectionResultFile; // file to write detection results, needed only for test mode
	std::string m_strConfigFile;

	// one detector for each camera
	std::vector<CarDetector> m_detectors;
};

#endif // CarDetectorApp_H
