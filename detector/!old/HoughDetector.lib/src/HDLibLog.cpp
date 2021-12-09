#include "HDLibLog.h"

INITIALIZE_EASYLOGGINGPP

void HDInitializeLogger() {
	static bool isLoggerInitialized = false;
	if (!isLoggerInitialized) {
		HDLOG_BEGIN(HDDETECT_LOGGER, 0, static_cast<const char**>(nullptr));
		isLoggerInitialized = true;
	}
}
