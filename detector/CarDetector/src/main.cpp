/*
 * CarDetector.
 * Detects parked cars on input image or video and calculates distance between them.
 * Writes information about available free parking places into a database.
 */

#include "CarDetectorApp.h"
#include "CarDetectorLog.h"

#include "TFException.h"

INITIALIZE_EASYLOGGINGPP

namespace {

std::vector<std::string> collectArgs(int argc, char** argv) {
	assert(argc >= 0);
	std::vector<std::string> args;
	args.reserve(static_cast<size_t>(argc));
	for (int i = 0; i < argc; ++i)
		args.emplace_back(argv[i]);
	return args;
}

} // anonymous namespace

int main(int argc, char** argv) {
	try {
		HDLOG_BEGIN(CDAPP_LOGGER, argc, argv);
		HDLOG << "Welcome to CarDetector app.";
		CarDetectorApp app;
		return app.run(collectArgs(argc, argv));
	} catch (TFException & _ex) {
		LOG(ERROR) << "TFException caught: " << _ex.message();
		return 1;
	} catch (const std::exception & _ex) {
		LOG(ERROR) << "std::exception caught: " << _ex.what();
		return 1;
	} catch (...) {
		LOG(ERROR) << "Unknown exception caught! Exit from app..";
		return 1;
	}
}
