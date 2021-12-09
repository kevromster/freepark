#ifndef HDTrainer_H
#define HDTrainer_H

#include "IHDTrainer.h"

#include "HoughForest.h"
#include "TrainOptions.h"
#include "PatchStorage.h"

class HDTrainer : public IHDTrainer
{
public:
	HDTrainer(const std::string & _strConfigFile);

	void train();
	void showTrainedLeafs();

private:
	void _loadTrainedForest();

	void _extractPatches();
	void _trainForest();

private:
	TrainOptions m_options;
	PatchStorage m_patchStorage;
	HoughForest m_forest;
	bool m_bTrainedForestLoaded;
};

#endif // HDTrainer_H
