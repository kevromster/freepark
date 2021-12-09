#ifndef HDLibLog_H
#define HDLibLog_H

#include "HDLog.h"

// This logger is used for printing normal application process information to enduser.
#define HDDETECT_LOGGER "hddetect_logger"
#define HDLOG CLOG(INFO, HDDETECT_LOGGER)

void HDInitializeLogger();

#endif // HDLibLog_H
