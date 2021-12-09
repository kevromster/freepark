#ifndef HoughTreeTrainer_H
#define HoughTreeTrainer_H

#include "HoughTree.h"

class PatchStorage;
class ImagePatch;

class HoughTreeTrainer {
public:
	HoughTreeTrainer(
		unsigned int _uMaxDepth,
		unsigned int _uMinPatchesInLeaf,
		unsigned int _uTrainIterationsCount,
		const PatchStorage & _patchStorage
	);

	std::unique_ptr<HoughTree> trainTree();

private:
	struct TrainPatches;
	struct PatchIndex;

	enum MEASURE_MODE {
		mmClassification,
		mmRegression
	};

	void _growTree(const TrainPatches & _patches, unsigned int _uCurNode, unsigned int _uCurDepth);
	void _makeLeaf(const TrainPatches & _patches, unsigned int _uCurNode);
	bool _trySplit(const TrainPatches & _patches, TrainPatches & _setA, TrainPatches & _setB, HoughNode::VotingData & _nodeVotingData, MEASURE_MODE _measureMode);

	void _split(
		const std::vector<const ImagePatch*> & _patches,
		const std::vector<PatchIndex> & _bestPatches,
		int _nThreshold,
		std::vector<const ImagePatch*> & _setA,
		std::vector<const ImagePatch*> & _setB
	) const;

	double _getSplitQuality(const TrainPatches & _setA, const TrainPatches & _setB, MEASURE_MODE _uMeasureMode) const;
	double _distMean(const std::vector<const ImagePatch*> & _setA, const std::vector<const ImagePatch*> & _setB) const;
	double _infGain(const TrainPatches & _setA, const TrainPatches & _setB) const;

	double _calcMeanDistance(const std::vector<const ImagePatch*> & _set) const;
	double _calcNegativeEntropy(const TrainPatches & _set) const;

	void _generateTest(HoughNode::VotingData & _data, unsigned int _uMaxWidth, unsigned int _uMaxHeight, unsigned int _uMaxChannelsCount) const;
	std::vector<PatchIndex> _findBestPatches(const HoughNode::VotingData  & _data, const std::vector<const ImagePatch*> & _patches);

	void _findThreshold(const std::vector<PatchIndex> & _positivePatches, const std::vector<PatchIndex> & _negativePatches, int & _nMinThreshold, int & _nMaxThreshold) const;
	void _fillThresholdBoundaries(const std::vector<PatchIndex> & _patches, int & _nMinThreshold, int & _nMaxThreshold) const;

private:
	const PatchStorage & m_patchStorage;
	HoughTree * m_pTrainingTree;  // active while training some tree

	unsigned int m_uMaxTreeDepth;
	unsigned int m_uMinPatchesInLeaf;  // stop tree growing when patches count is less than this value

	const unsigned int m_uTrainIterationsCount;
	const double m_dPosNegPatchesRatio;
};

#endif // HoughTreeTrainer_H
