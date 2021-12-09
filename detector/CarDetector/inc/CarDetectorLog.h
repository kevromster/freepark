#ifndef CarDetectorLog_H
#define CarDetectorLog_H

#include <easylogging++.h>

#define HDLOG_BEGIN(logger, argc, argv)\
  START_EASYLOGGINGPP(argc, argv);\
  el::Loggers::reconfigureAllLoggers(el::ConfigurationType::Format, "[%datetime{%H:%m:%s}] %level: %msg");\
  el::Configurations configuration;\
  configuration.setToDefault();\
  configuration.set(el::Level::Info, el::ConfigurationType::Format, "[%datetime{%H:%m:%s}] %msg");\
  el::Loggers::getLogger(logger)->configure(configuration);

#define CDAPP_LOGGER "cdapp_logger"
#define HDLOG CLOG(INFO, CDAPP_LOGGER)

#endif // CarDetectorLog_H
