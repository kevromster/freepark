#include "HoughForest.h"
#include "HoughTreeTrainer.h"
#include "HDException.h"

#include <fstream>

namespace {
const unsigned int g_uStorageVersion = 1;
}

void HoughForest::initialize(unsigned int _uTreesCount) {
	m_trees.clear();
	m_trees.resize(_uTreesCount);
}

void HoughForest::train(unsigned int _uMaxDepth, unsigned int _uMinPatchesInLeaf, unsigned int _uTrainIterationsCount, const PatchStorage & _patchStorage) {
	HoughTreeTrainer trainer(_uMaxDepth, _uMinPatchesInLeaf, _uTrainIterationsCount, _patchStorage);

	for (auto & pTree : m_trees)
		pTree = trainer.trainTree();
}

void HoughForest::save(const std::string & _strFileName) const {
	std::ofstream stream(_strFileName);
	if (!stream.is_open())
		throw HDIOException("Error opening file for forest saving: \"" + _strFileName + "\"");

	stream << g_uStorageVersion << " " << m_trees.size() << std::endl;

	for (const auto & pTree : m_trees) {
		assert(pTree);
		pTree->save(stream);
		stream << std::endl;
	}
}

void HoughForest::load(const std::string & _strFileName) {
	std::ifstream stream(_strFileName);
	if (!stream.is_open())
		throw HDIOException("Error opening file for forest loading: \"" + _strFileName + "\"");

	unsigned int uVersion = 0;
	stream >> uVersion;

	if (uVersion != g_uStorageVersion)
		throw BadVersionException(uVersion, g_uStorageVersion);

	unsigned int uTreesCount = 0;
	stream >> uTreesCount;
	std::vector<std::unique_ptr<HoughTree>> trees;
	trees.reserve(uTreesCount);

	for (unsigned int uTreeIdx = 0; uTreeIdx < uTreesCount; ++uTreeIdx) {
		std::unique_ptr<HoughTree> pTree = std::make_unique<HoughTree>();
		pTree->load(stream);
		trees.push_back(std::move(pTree));
	}

	m_trees.swap(trees);
}

void HoughForest::showLeafs(unsigned int _uTreeIdx) const {
	if (_uTreeIdx < m_trees.size())
		m_trees[_uTreeIdx]->showLeafs();
}

std::vector<const HoughLeaf*> HoughForest::regression(int _nBaseX, int _nBaseY, const std::vector<std::unique_ptr<cv::Mat>> & _imageChannels) const {
	std::vector<const HoughLeaf*> result;
	result.reserve(m_trees.size());

	for (const auto & tree : m_trees)
		result.push_back(tree->regression(_nBaseX, _nBaseY, _imageChannels));

	return result;
}
