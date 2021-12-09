#include "HoughTreeTrainer.h"
#include "PatchStorage.h"

#include <opencv2/highgui.hpp>

namespace {

template<typename T>
class WeakPtrHolder {
public:
	WeakPtrHolder(T *& _ptr) : m_ptr(_ptr) {}
	~WeakPtrHolder() {m_ptr = nullptr;}

private:
	T *& m_ptr;
};

} // anonymous namespace

struct HoughTreeTrainer::TrainPatches {
	bool isEmpty() const {return getTotalSize() == 0;}
	size_t getTotalSize() const {return (m_positivePatches.size() + m_negativePatches.size());}

	std::vector<const ImagePatch*> m_positivePatches;
	std::vector<const ImagePatch*> m_negativePatches;
};

struct HoughTreeTrainer::PatchIndex {
	PatchIndex() : m_nValue(0), m_uPatchIndex(0) {}
	PatchIndex(int _nValue, unsigned int _uPatchIndex) : m_nValue(_nValue), m_uPatchIndex(_uPatchIndex) {}

	bool operator <(const PatchIndex & _other) const {return m_nValue < _other.m_nValue;}

	int m_nValue;
	unsigned int m_uPatchIndex;
};

HoughTreeTrainer::HoughTreeTrainer(
	unsigned int _uMaxDepth,
	unsigned int _uMinPatchesInLeaf,
	unsigned int _uTrainIterationsCount,
	const PatchStorage & _patchStorage
) :
	m_patchStorage(_patchStorage),
	m_pTrainingTree(nullptr),
	m_uMaxTreeDepth(_uMaxDepth),
	m_uMinPatchesInLeaf(_uMinPatchesInLeaf),
	m_uTrainIterationsCount(_uTrainIterationsCount),
	m_dPosNegPatchesRatio(_patchStorage.getPositiveNegativeRatio())
{}

std::unique_ptr<HoughTree> HoughTreeTrainer::trainTree() {
	std::unique_ptr<HoughTree> pTree = std::make_unique<HoughTree>();
	WeakPtrHolder<HoughTree> ptrHolder(m_pTrainingTree = pTree.get());
	m_pTrainingTree->initialize(m_uMaxTreeDepth);

	TrainPatches trainingPatches;
	trainingPatches.m_positivePatches.reserve(m_patchStorage.getPositivePatches().size());
	trainingPatches.m_negativePatches.reserve(m_patchStorage.getNegativePatches().size());

	for (const auto & patch : m_patchStorage.getPositivePatches())
		trainingPatches.m_positivePatches.push_back(&patch);

	for (const auto & patch : m_patchStorage.getNegativePatches())
		trainingPatches.m_negativePatches.push_back(&patch);

	assert(trainingPatches.m_positivePatches.size() > 0);
	assert(trainingPatches.m_negativePatches.size() > 0);

	_growTree(trainingPatches, 0, 0);
	return pTree;
}

void HoughTreeTrainer::_growTree(const TrainPatches & _patches, unsigned int _uCurNode, unsigned int _uCurDepth) {
	// if only negative patches left or we reached maximum depth, stop growing
	if(_uCurDepth >= m_uMaxTreeDepth || _patches.m_positivePatches.size() == 0) {
		_makeLeaf(_patches, _uCurNode);
		return;
	}

	MEASURE_MODE measureMode = mmRegression;
	if (double(_patches.m_negativePatches.size()) / double(_patches.m_negativePatches.size() + _patches.m_positivePatches.size()) >= 0.05 && _uCurDepth < m_uMaxTreeDepth - 2)
		measureMode = m_patchStorage.getRandomGenerator().next() % 2 ? mmRegression : mmClassification;

	TrainPatches setA;
	TrainPatches setB;
	HoughNode::VotingData nodeVotingData;

	if (!_trySplit(_patches, setA, setB, nodeVotingData, measureMode)) {
		_makeLeaf(_patches, _uCurNode);
		return;
	}

	m_pTrainingTree->setNode(_uCurNode, HoughNode(nodeVotingData));

	// Go left: if enough patches are left, continue growing
	if (setA.m_negativePatches.size() + setA.m_positivePatches.size() > m_uMinPatchesInLeaf)
		_growTree(setA, (2*_uCurNode) + 1, _uCurDepth + 1);
	else
		_makeLeaf(setA, (2*_uCurNode) + 1);

	// Go right: if enough patches are left, continue growing
	if (setB.m_negativePatches.size() + setB.m_positivePatches.size() > m_uMinPatchesInLeaf)
		_growTree(setB, (2*_uCurNode) + 2, _uCurDepth + 1);
	else
		_makeLeaf(setB, (2*_uCurNode) + 2);
}

void HoughTreeTrainer::_makeLeaf(const TrainPatches & _patches, unsigned int _uCurNode) {
	const double dForegroundProbability = double(_patches.m_positivePatches.size()) / double((m_dPosNegPatchesRatio * _patches.m_negativePatches.size()) + _patches.m_positivePatches.size());
	std::unique_ptr<HoughLeaf> leaf = std::make_unique<HoughLeaf>(dForegroundProbability, _patches.m_positivePatches.size());

	for (const auto & patch : _patches.m_positivePatches)
		leaf->addPatchCenter(patch->getCenterVector().getVector());

	m_pTrainingTree->setNode(_uCurNode, HoughNode(leaf));
}

bool HoughTreeTrainer::_trySplit(
	const TrainPatches & _patches,
	TrainPatches & _setA,
	TrainPatches & _setB,
	HoughNode::VotingData & _nodeVotingData,
	MEASURE_MODE _measureMode
) {
	bool bFound = false;

	// temporary data for split patches into two sets
	TrainPatches tmpA;
	TrainPatches tmpB;

	HoughNode::VotingData generatedTest;
	std::vector<PatchIndex> bestPositivePatches;
	std::vector<PatchIndex> bestNegativePatches;

	double dBestSplitQuality = -DBL_MAX;
	const cv::Size patchSize = _patches.m_positivePatches[0]->getBBox().size();  // assume all patches have the same bbox size

	// find best test within specified iterations
	for(unsigned int i = 0; i < m_uTrainIterationsCount; ++i) {

		// reset temporary data for split
		tmpA.m_positivePatches.clear();
		tmpA.m_negativePatches.clear();
		tmpB.m_positivePatches.clear();
		tmpB.m_negativePatches.clear();

		// generate binary test without threshold
		_generateTest(generatedTest, patchSize.width, patchSize.height, PATCH_CHANNELS_COUNT);

		// compute value for each patch and find threshold
		 bestPositivePatches = _findBestPatches(generatedTest, _patches.m_positivePatches);
		 bestNegativePatches = _findBestPatches(generatedTest, _patches.m_negativePatches);

		int nMinThreshold = 0;
		int nMaxThreshold = 0;
		_findThreshold(bestPositivePatches, bestNegativePatches, nMinThreshold, nMaxThreshold);
		const int nThreshold = nMaxThreshold - nMinThreshold;

		if (nThreshold <= 0)
			continue;

		// find best threshold for splitting
		for(unsigned int j = 0; j < 10; ++j) {
			const int nRandomThreshold = nMinThreshold + (m_patchStorage.getRandomGenerator().next() % nThreshold);

			_split(_patches.m_negativePatches, bestNegativePatches, nRandomThreshold, tmpA.m_negativePatches, tmpB.m_negativePatches);
			_split(_patches.m_positivePatches, bestPositivePatches, nRandomThreshold, tmpA.m_positivePatches, tmpB.m_positivePatches);

			// avoid empty sets
			if (tmpA.isEmpty() || tmpB.isEmpty())
				continue;

			const double dSplitQuality = _getSplitQuality(tmpA, tmpB, _measureMode);

			if(dSplitQuality > dBestSplitQuality) {
				bFound = true;
				dBestSplitQuality = dSplitQuality;

				_nodeVotingData = generatedTest;
				_nodeVotingData.m_nThreshold = nRandomThreshold;

				_setA = std::move(tmpA);
				_setB = std::move(tmpB);
			}
		}
	}

	return bFound;
}

void HoughTreeTrainer::_findThreshold(const std::vector<PatchIndex> & _positivePatches, const std::vector<PatchIndex> & _negativePatches, int & _nMinThreshold, int & _nMaxThreshold) const {
	_nMinThreshold = INT_MAX;
	_nMaxThreshold = INT_MIN;

	_fillThresholdBoundaries(_negativePatches, _nMinThreshold, _nMaxThreshold);
	_fillThresholdBoundaries(_positivePatches, _nMinThreshold, _nMaxThreshold);
}

void HoughTreeTrainer::_fillThresholdBoundaries(const std::vector<PatchIndex> & _patches, int & _nMinThreshold, int & _nMaxThreshold) const {
	if (_patches.size() > 0) {
		// patches are assumed to be sorted
		if (_nMinThreshold > _patches.front().m_nValue)
			_nMinThreshold = _patches.front().m_nValue;
		if (_nMaxThreshold < _patches.back().m_nValue)
			_nMaxThreshold = _patches.back().m_nValue;
	}
}

void HoughTreeTrainer::_generateTest(HoughNode::VotingData & _data, unsigned int _uMaxWidth, unsigned int _uMaxHeight, unsigned int _uMaxChannelsCount) const {
	_data.m_uX1 = m_patchStorage.getRandomGenerator().next() % _uMaxWidth;
	_data.m_uY1 = m_patchStorage.getRandomGenerator().next() % _uMaxHeight;
	_data.m_uX2 = m_patchStorage.getRandomGenerator().next() % _uMaxWidth;
	_data.m_uY2 = m_patchStorage.getRandomGenerator().next() % _uMaxHeight;
	_data.m_uChannel = m_patchStorage.getRandomGenerator().next() % _uMaxChannelsCount;
}

std::vector<HoughTreeTrainer::PatchIndex> HoughTreeTrainer::_findBestPatches(const HoughNode::VotingData  & _data, const std::vector<const ImagePatch*> & _patches) {
	std::vector<HoughTreeTrainer::PatchIndex> bestPatches;
	bestPatches.reserve(_patches.size());

	for (unsigned int uIdx = 0; uIdx < _patches.size(); ++uIdx) {
		const cv::Mat & channelPatch = _patches[uIdx]->getPatch(_data.m_uChannel);

		const int p1 = channelPatch.at<uchar>(cv::Point(_data.m_uX1, _data.m_uY1));
		const int p2 = channelPatch.at<uchar>(cv::Point(_data.m_uX2, _data.m_uY2));

		bestPatches.emplace_back(p1 - p2, uIdx);
	}

	std::sort(bestPatches.begin(), bestPatches.end());
	return bestPatches;
}

void HoughTreeTrainer::_split(
	const std::vector<const ImagePatch*> & _patches,
	const std::vector<PatchIndex> & _bestPatches,
	int _nThreshold,
	std::vector<const ImagePatch*> & _setA,
	std::vector<const ImagePatch*> & _setB
) const {
	// search largest value less than threshold
	auto it = _bestPatches.begin();
	while (it != _bestPatches.end() && it->m_nValue < _nThreshold)
		++it;

	_setA.resize(it - _bestPatches.begin());
	_setB.resize(_patches.size() - _setA.size());

	it = _bestPatches.begin();
	for(size_t i = 0; i < _setA.size(); ++i, ++it)
		_setA[i] = _patches[it->m_uPatchIndex];

	it = _bestPatches.begin() + _setA.size();
	for(size_t i = 0; i < _setB.size(); ++i, ++it)
		_setB[i] = _patches[it->m_uPatchIndex];
}

double HoughTreeTrainer::_getSplitQuality(const TrainPatches & _setA, const TrainPatches & _setB, MEASURE_MODE _measureMode) const {
	if (_measureMode == mmClassification)
		return _infGain(_setA, _setB);

	return -_distMean(_setA.m_positivePatches, _setB.m_positivePatches);
}

double HoughTreeTrainer::_distMean(const std::vector<const ImagePatch*> & _setA, const std::vector<const ImagePatch*> & _setB) const {
	const double dDistanceA = _calcMeanDistance(_setA);
	const double dDistanceB = _calcMeanDistance(_setB);

	return (dDistanceA + dDistanceB)/double(_setA.size() + _setB.size());
}

double HoughTreeTrainer::_calcMeanDistance(const std::vector<const ImagePatch*> & _set) const {
	double dMeanX = 0.;
	double dMeanY = 0.;

	for(const auto & patch : _set) {
		dMeanX += patch->getCenterVector().getVector().x;
		dMeanY += patch->getCenterVector().getVector().y;
	}

	dMeanX /= double(_set.size());
	dMeanY /= double(_set.size());

	double dDistance = 0.;

	for(const auto & patch : _set) {
		double tmp = patch->getCenterVector().getVector().x - dMeanX;
		dDistance += (tmp*tmp);
		tmp = patch->getCenterVector().getVector().y - dMeanY;
		dDistance += (tmp*tmp);
	}

	return dDistance;
}

double HoughTreeTrainer::_infGain(const TrainPatches & _setA, const TrainPatches & _setB) const {
	const double dSizeA = _setA.getTotalSize();
	const double dSizeB = _setB.getTotalSize();

	const double dNegativeEntropyA = _calcNegativeEntropy(_setA);
	const double dNegativeEntropyB = _calcNegativeEntropy(_setB);

	return ((dSizeA*dNegativeEntropyA) + (dSizeB*dNegativeEntropyB)) / (dSizeA + dSizeB);
}

double HoughTreeTrainer::_calcNegativeEntropy(const TrainPatches & _set) const {
	const double dSize = _set.getTotalSize();

	// negative entropy: sum p*log(p)
	double dNegativeEntropy = 0.;

	double p = double(_set.m_negativePatches.size())/dSize;
	if (p > 0)
		dNegativeEntropy += p*log(p);

	p = double(_set.m_positivePatches.size())/dSize;
	if (p > 0)
		dNegativeEntropy += p*log(p);

	return dNegativeEntropy;
}
