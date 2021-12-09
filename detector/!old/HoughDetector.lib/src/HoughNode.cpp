#include "HoughNode.h"

#include <fstream>

void HoughNode::VotingData::save(std::ofstream & _stream) const {
	_stream << m_uX1 << " " << m_uY1 << " " << m_uX2 << " " << m_uY2 << " " << m_uChannel << " " << m_nThreshold;
}

void HoughNode::VotingData::load(std::ifstream & _stream) {
	VotingData data;
	_stream >> data.m_uX1;
	_stream >> data.m_uY1;
	_stream >> data.m_uX2;
	_stream >> data.m_uY2;
	_stream >> data.m_uChannel;
	_stream >> data.m_nThreshold;

	std::swap(data, *this);
}

HoughNode::HoughNode(const VotingData & _votingData) :
	m_voting(_votingData)
{}

HoughNode::HoughNode(std::unique_ptr<HoughLeaf> & _leaf) :
	m_leaf(std::move(_leaf))
{}

void HoughNode::showLeaf() const {
	if (m_leaf)
		m_leaf->showPatchCenters();
}

void HoughNode::save(std::ofstream & _stream) const {
	_stream << isLeaf() << " ";

	if (isLeaf())
		getLeaf()->save(_stream);
	else
		getVotingData().save(_stream);
}

void HoughNode::load(std::ifstream & _stream) {
	bool bIsLeaf = false;
	_stream >> bIsLeaf;

	if (bIsLeaf) {
		std::unique_ptr<HoughLeaf> pLeaf = std::make_unique<HoughLeaf>();
		pLeaf->load(_stream);
		m_leaf = std::move(pLeaf);
	} else {
		HoughNode::VotingData votingData;
		votingData.load(_stream);
		std::swap(votingData, m_voting);
	}
}
