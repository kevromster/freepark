#include "HoughTree.h"

#include <fstream>

void HoughTree::initialize(unsigned int _uMaxDepth) {
	// maximum number of nodes: 2^(MaxDepth+1)-1
	const unsigned int uMaxNodesCount = static_cast<unsigned int>(std::pow(2., _uMaxDepth + 1)) - 1;
	m_nodes.resize(uMaxNodesCount);
}

void HoughTree::setNode(unsigned int _uNodeIdx, HoughNode && _node) {
	assert(_uNodeIdx < m_nodes.size());
	m_nodes[_uNodeIdx] = std::move(_node);
}

void HoughTree::save(std::ofstream & _stream) const {
	_stream << m_nodes.size() << std::endl;

	for (const HoughNode & node : m_nodes) {
		node.save(_stream);
		_stream << std::endl;
	}
}

void HoughTree::load(std::ifstream & _stream) {
	unsigned int uNodesCount = 0;
	_stream >> uNodesCount;

	std::vector<HoughNode> nodes;
	nodes.reserve(uNodesCount);

	for (unsigned int uNodeIdx = 0; uNodeIdx < uNodesCount; ++uNodeIdx) {
		HoughNode node;
		node.load(_stream);
		nodes.push_back(std::move(node));
	}

	m_nodes.swap(nodes);
}

void HoughTree::showLeafs() const {
	for (const auto & node : m_nodes)
		node.showLeaf();
}

const HoughLeaf * HoughTree::regression(int _nBaseX, int _nBaseY, const std::vector<std::unique_ptr<cv::Mat>> & _imageChannels) const {
	assert(m_nodes.size() > 0);

	unsigned int uNodeIdx = 0;
	const HoughNode * pNode = &m_nodes[uNodeIdx];

	while (!pNode->isLeaf()) {
		// binary test 0 - left, 1 - right
		const HoughNode::VotingData & nodeData = pNode->getVotingData();
		assert(nodeData.m_uChannel < _imageChannels.size());

		// test pixels
		const int p1 = _imageChannels[nodeData.m_uChannel]->at<uchar>(cv::Point(_nBaseX + nodeData.m_uX1, _nBaseY + nodeData.m_uY1));
		const int p2 = _imageChannels[nodeData.m_uChannel]->at<uchar>(cv::Point(_nBaseX + nodeData.m_uX2, _nBaseY + nodeData.m_uY2));
		const bool bTest = (p1 - p2) >= nodeData.m_nThreshold;

		// go to next node
		unsigned int uNextIdx = uNodeIdx + 1 + bTest;
		uNodeIdx += uNextIdx;

		assert(uNodeIdx < m_nodes.size());
		pNode = &m_nodes[uNodeIdx];
	}

	// return leaf
	return pNode->getLeaf();
}
