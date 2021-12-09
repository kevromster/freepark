#ifndef HoughForestOptions_H
#define HoughForestOptions_H

#include <opencv2/core.hpp>

/**
 * Options for HoughForest algorithm.
 */
class HoughForestOptions
{
public:
	HoughForestOptions() :
		m_uPatchesCountToExtract(0),
		m_uTreesCount(0),
		m_uTreeMaxDepth(0),
		m_uMinPatchesInLeaf(0),
		m_uTrainIterationsCount(0)
	{}
	virtual ~HoughForestOptions() {}

	virtual bool areCorrect() const {
		return
			m_patchSize.width > 0 &&
			m_patchSize.height > 0 &&
			m_trainObjectSize.width > 0 &&
			m_trainObjectSize.height > 0 &&
			m_uPatchesCountToExtract > 0 &&
			m_uTreesCount > 0 &&
			m_uTreeMaxDepth > 0 &&
			m_uMinPatchesInLeaf > 0 &&
			m_uTrainIterationsCount > 0 &&
			!m_strTrainedForestPath.empty();
	}

	const cv::Size & getPatchSize() const {return m_patchSize;}
	const cv::Size & getTrainObjectSize() const {return m_trainObjectSize;}
	unsigned int getPatchesCount() const {return m_uPatchesCountToExtract;}
	unsigned int getTreesCount() const {return m_uTreesCount;}

	unsigned int getTreeMaxDepth() const {return m_uTreeMaxDepth;}
	unsigned int getMinPatchesInLeaf() const {return m_uMinPatchesInLeaf;}
	unsigned int getTrainIterationsCount() const {return m_uTrainIterationsCount;}

	const std::string & getTrainedForestPath() const {return m_strTrainedForestPath;}

	void setPatchWidth(int _nWidth) {m_patchSize.width = _nWidth;}
	void setPatchHeight(int _nHeight) {m_patchSize.height = _nHeight;}
	void setTrainObjectWidth(int _nWidth) {m_trainObjectSize.width = _nWidth;}
	void setTrainObjectHeight(int _nHeight) {m_trainObjectSize.height = _nHeight;}
	void setPatchesCount(unsigned int _uCount) {m_uPatchesCountToExtract = _uCount;}
	void setTreesCount(unsigned int _uCount) {m_uTreesCount = _uCount;}

	void setTreeMaxDepth(unsigned int _uValue) {m_uTreeMaxDepth = _uValue;}
	void setMinPatchesInLeaf(unsigned int _uValue) {m_uMinPatchesInLeaf = _uValue;}
	void setTrainIterationsCount(unsigned int _uValue) {m_uTrainIterationsCount = _uValue;}

	void setTrainedForestPath(const std::string & _path) {m_strTrainedForestPath = _path;}

private:
	cv::Size m_patchSize;
	cv::Size m_trainObjectSize;

	unsigned int m_uPatchesCountToExtract;
	unsigned int m_uTreesCount;
	unsigned int m_uTreeMaxDepth;
	unsigned int m_uMinPatchesInLeaf;
	unsigned int m_uTrainIterationsCount;

	std::string m_strTrainedForestPath;
};

#endif // HoughForestOptions_H
