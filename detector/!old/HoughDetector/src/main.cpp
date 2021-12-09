#include "HDApp.h"
#include "HDAppLog.h"
#include "HDException.h"

INITIALIZE_EASYLOGGINGPP

namespace {

const int g_nResultSuccess = 0;
const int g_nResultError = 1;

int usage() {
	HDLOG << "Usage: HoughDetector <config-file> [--train | --detect <image-file> <out-file>]";
	return g_nResultError;
}

int train(const std::string & _strConfigFile) {
	HDApp app(_strConfigFile);
	app.train();
	return g_nResultSuccess;
}

int detect(const std::string & _strConfigFile, const std::string & _strImageFile, const std::string & _strOutFile) {
	HDApp app(_strConfigFile);
	app.detect(_strImageFile, _strOutFile);
	return g_nResultSuccess;
}

} // anonymous namespace

int main(int argc, char** argv) {
	try {
		HDLOG_BEGIN(HDAPP_LOGGER, argc, argv);
		HDLOG << "Welcome to HoughDetector app.";

		if (argc != 3 && argc != 5)
			return usage();

		const std::string strConfigFile = argv[1];
		const std::string strOption = argv[2];

		if (strOption == "--train") {
			return train(strConfigFile);
		}

		if (strOption == "--detect") {
			return argc == 5 ? detect(strConfigFile, std::string(argv[3]), std::string(argv[4])) : usage();
		}

		return usage();
	} catch (HDException & _ex) {
		LOG(ERROR) << "HDException caught: " << _ex.message();
		return g_nResultError;
	} catch (const std::exception & _ex) {
		LOG(ERROR) << "std::exception caught: " << _ex.what();
		return g_nResultError;
	} catch (...) {
		LOG(ERROR) << "Unknown exception caught! Exit from app..";
		return g_nResultError;
	}
}
