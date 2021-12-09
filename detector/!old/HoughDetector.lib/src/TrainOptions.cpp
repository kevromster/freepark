#include "TrainOptions.h"

#include "HDLibLog.h"
#include "TrainOptionsLoader.h"

TrainOptions TrainOptions::createFromFile(const std::string & _strConfigFile) {
	HDLOG << "Loading options from file...";
	TrainOptions options;
	TrainOptionsLoader loader(_strConfigFile, options);
	loader.loadAll();
	HDLOG << "Options have been loaded successfully.";
	return options;
}
