#ifndef AI4AllDetectorApp_H
#define AI4AllDetectorApp_H

#include "SubmitCameraItem.h"
#include "AI4AllDetector.h"

#include <string>
#include <vector>

/**
 * The application instance.
 */
class AI4AllDetectorApp
{
public:
	AI4AllDetectorApp();

	/**
	 * Runs application with the specified arguments.
	 *
	 * @param _args command line input arguiments
	 * @return the result code
	 */
	int run(const std::vector<std::string> & _args);

private:
	AI4AllDetector m_detector;
};

#endif // AI4AllDetectorApp_H
