/*
 * AI4AllDetector.
 * Checks www.ai4all.ru server for new SubmitCameraItems and runs detection on them.
 */

#include "AI4AllDetectorApp.h"
#include "AI4AllDetectorLog.h"
#include "TFException.h"

INITIALIZE_EASYLOGGINGPP

namespace {

std::vector<std::string> collectArgs(int argc, char** argv) {
	std::vector<std::string> args;
	if (argc > 0) {
		args.reserve(static_cast<size_t>(argc));
		for (int i = 0; i < argc; ++i)
			args.emplace_back(argv[i]);
	}
	return args;
}

} // anonymous namespace

int main(int argc, char** argv) {
	try {
		HDLOG_BEGIN(CDAPP_LOGGER, argc, argv);
		HDLOG << "Welcome to AI4AllDetector app.";
		AI4AllDetectorApp app;
		return app.run(collectArgs(argc, argv));
	} catch (const TFException & _ex) {
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
