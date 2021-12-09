#include "HDTrainer.h"

#include "HDLibLog.h"

std::unique_ptr<IHDTrainer> IHDTrainer::createTrainer(const std::string & _strConfigFile) {
	HDInitializeLogger();
	return std::make_unique<HDTrainer>(_strConfigFile);
}

HDTrainer::HDTrainer(const std::string & _strConfigFile) :
	m_options(TrainOptions::createFromFile(_strConfigFile)),
	m_bTrainedForestLoaded(false)
{
}

void HDTrainer::train() {
	HDLOG << "Extracting patches...";
	_extractPatches();
	HDLOG << "Training forest...";
	_trainForest();
	HDLOG << "Forest trained successfully." ;
}

void HDTrainer::showTrainedLeafs() {
	_loadTrainedForest();
	m_forest.showLeafs(0);
}

void HDTrainer::_loadTrainedForest() {
	if (m_bTrainedForestLoaded) {
		HDLOG << "Trained forest has been already loaded.";
		return;
	}
	HDLOG << "Loading forest...";
	m_forest.load(m_options.getTrainedForestPath());
	m_bTrainedForestLoaded = true;
	HDLOG << "Forest has been loaded.";
}

void HDTrainer::_extractPatches() {
	m_patchStorage.initialize(m_options.getPatchSize(), m_options.getPatchesCount());

	for (const TrainImageInfo & imageInfo : m_options.getPositiveTrainImages())
		m_patchStorage.extractPositivePatches(imageInfo);

	for (const std::string & negativeImageFileName : m_options.getNegativeTrainImages())
		m_patchStorage.extractNegativePatches(negativeImageFileName);
}

void HDTrainer::_trainForest() {
	m_forest.initialize(m_options.getTreesCount());
	m_forest.train(m_options.getTreeMaxDepth(), m_options.getMinPatchesInLeaf(), m_options.getTrainIterationsCount(), m_patchStorage);
	m_forest.save(m_options.getTrainedForestPath());
}
