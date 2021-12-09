#include "CarDetectorApp.h"

#include "CameraOptions.h"
#include "CarDetector.h"
#include "CarDetectorLog.h"

#include <thread>

namespace {

// Returns duration in seconds from the moment specified up to the current time.
long getDurationFromMoment(const std::chrono::time_point<std::chrono::high_resolution_clock> & _moment) {
	return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _moment).count();
}

} // anonymous namespace

CarDetectorApp::CarDetectorApp() :
	m_bMultiMode(false),
	m_bTestMode(false)
{}

int CarDetectorApp::run(const std::vector<std::string> & _args) {
	if (!_parseArgs(_args))
		return 1;

	if (m_bMultiMode)
		HDLOG << "The application runs in multi mode.";
	if (m_bTestMode)
		HDLOG << "The application runs in test mode, the results will be saved into '" << m_strDetectionResultFile << "'.";

	_initializeDetectors(_readCameraOptions());
	_runDetectors();

	return 0;
}

std::vector<CameraOptions> CarDetectorApp::_readCameraOptions() const {
	if (m_bMultiMode)
		return CameraOptions::loadMultiConfig(m_strConfigFile);

	std::vector<CameraOptions> options;
	options.emplace_back(m_strConfigFile);
	return options;
}

bool CarDetectorApp::_parseArgs(const std::vector<std::string> & _args) {
	if (_args.size() != 2 && _args.size() != 3 && _args.size() != 4) {
		_printUsage();
		return false;
	}

	// skip first argument - its a name of the application
	for (auto it = _args.begin() + 1; it != _args.end(); ++it) {
		if (*it == "--help" || *it == "-h") {
			_printUsage();
			return false;
		}
		if (*it == "--multi") {
			m_bMultiMode = true;
			continue;
		}
		if (*it == "--test") {
			m_bTestMode = true;
			++it;
			if (it == _args.end()) {
				_printUsage();
				return false;
			}
			m_strDetectionResultFile = *it;
			continue;
		}
		m_strConfigFile = *it;
	}

	if (m_strConfigFile.empty()) {
		LOG(ERROR) << "No config file specified.";
		_printUsage();
		return false;
	}

	if (m_bTestMode && m_strDetectionResultFile.empty()) {
		LOG(ERROR) << "No detection result file specified for the test mode.";
		_printUsage();
		return false;
	}

	return true;
}

void CarDetectorApp::_printUsage() const {
	HDLOG
		<< "Usage:" << std::endl
		<< "  CarDetector [options] <config-file>" << std::endl
		<< "    The application connects to cameras by URIs specified in configuration files," << std::endl
		<< "    takes screenshots and runs cars detection algorithm on them." << std::endl
		<< "    Then it sends information about found free parking places to freepark backend server." << std::endl
		<< std::endl
		<< "  Possible options:" << std::endl
		<< "    --multi" << std::endl
		<< "        The multi-configuration file is expected." << std::endl
		<< "    --test <detectionResultFile>" << std::endl
		<< "        The application runs in test mode. No data pushed to the backend, no infinite running in loop." << std::endl
		<< "        All detection results will be stored in specified <detectionResultFile>." << std::endl;
}

void CarDetectorApp::_initializeDetectors(const std::vector<CameraOptions> & _cameraOptions) {
	m_detectors.clear();
	m_detectors.reserve(_cameraOptions.size());

	for (const CameraOptions & opts : _cameraOptions)
		m_detectors.emplace_back(opts, m_bTestMode, m_strDetectionResultFile);
}

void CarDetectorApp::_runDetectors() {
	// store flags of skipped detectors here
	std::vector<char> isSkippedBecauseTooFast(m_detectors.size(), 0);

	// main application cycle
	for (std::size_t i = 0;; ++i) {
		if (i == m_detectors.size()) {
			if (m_bTestMode)
				break;  // run detectors only once for the test mode
			i = 0;
		}

		CarDetector & detectorToRun = m_detectors[i];

		// if all detectors were skipped (i.e. all flags are '1' in the vector), then sleep before next detector run
		if (std::find(isSkippedBecauseTooFast.begin(), isSkippedBecauseTooFast.end(), 0) == isSkippedBecauseTooFast.end()) {
			_sleepBeforeNextRun(detectorToRun);
		}

		// get elapsed time in seconds since last detector was run
		const long lElapsedSecs = getDurationFromMoment(detectorToRun.getLastStartDetectionTime());

		if (lElapsedSecs > 0 && static_cast<unsigned long>(lElapsedSecs) >= detectorToRun.getCameraProbeInterval()) {
			_runDetector(detectorToRun);
			isSkippedBecauseTooFast[i] = 0;
		} else
			isSkippedBecauseTooFast[i] = 1;
	}
}

void CarDetectorApp::_runDetector(CarDetector & _detector) {
	HDLOG << std::endl << "~~~~~ Run detector for camera " << _detector.getCameraURI();
	_detector.run();
}

void CarDetectorApp::_sleepBeforeNextRun(const CarDetector & _nextDetectorToRun) const {
	const long lElapsedSecs = getDurationFromMoment(_nextDetectorToRun.getLastStartDetectionTime());
	const long lToSleep = _nextDetectorToRun.getCameraProbeInterval() - lElapsedSecs;

	if (lToSleep > 0) {
		HDLOG << "CarDetectorApp: sleeping for " << lToSleep << " secs before next detector run...";
		std::this_thread::sleep_for(std::chrono::seconds(lToSleep));
		HDLOG << "CarDetectorApp: waked up";
	}
}
