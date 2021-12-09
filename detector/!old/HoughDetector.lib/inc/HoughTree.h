#ifndef HoughTree_H
#define HoughTree_H

#include "HoughNode.h"

class HoughTree
{
public:
	void initialize(unsigned int _uMaxDepth);
	void setNode(unsigned int _uNodeIdx, HoughNode && _node);

	void save(std::ofstream & _stream) const;
	void load(std::ifstream & _stream);

	void showLeafs() const;
	const HoughLeaf * regression(int _nBaseX, int _nBaseY, const std::vector<std::unique_ptr<cv::Mat>> & _imageChannels) const;

private:
	std::vector<HoughNode> m_nodes;
};

#endif // HoughTree_H
