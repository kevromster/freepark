#ifndef HoughForest_H
#define HoughForest_H

#include "HoughTree.h"

class PatchStorage;

class HoughForest
{
public:
	void initialize(unsigned int _uTreesCount);
	void train(unsigned int _uMaxDepth, unsigned int _uMinPatchesInLeaf, unsigned int _uTrainIterationsCount, const PatchStorage & _patchStorage);

	std::vector<const HoughLeaf*> regression(int _nBaseX, int _nBaseY, const std::vector<std::unique_ptr<cv::Mat>> & _imageChannels) const;

	void save(const std::string & _strFileName) const;
	void load(const std::string & _strFileName);

	void showLeafs(unsigned int _uTreeIdx) const;

private:
	std::vector<std::unique_ptr<HoughTree>> m_trees;
};

#endif // HoughForest_H
