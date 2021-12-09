#include "DetectOptions.h"

#include "HDLibLog.h"
#include "DetectOptionsLoader.h"

DetectOptions DetectOptions::createFromFile(const std::string & _strConfigFile) {
	HDLOG << "Loading detection options from file...";
	DetectOptions options;
	DetectOptionsLoader loader(_strConfigFile, options);
	loader.loadAll();
	HDLOG << "Options have been loaded successfully.";
	return options;
}
