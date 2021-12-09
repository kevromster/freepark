#ifndef HoughNode_H
#define HoughNode_H

#include "HoughLeaf.h"

#include <memory>

class HoughNode {
public:
	struct VotingData {
		VotingData() : m_uX1(0), m_uY1(0), m_uX2(0), m_uY2(0), m_uChannel(0), m_nThreshold(0) {}

		void save(std::ofstream & _stream) const;
		void load(std::ifstream & _stream);

		unsigned int m_uX1;
		unsigned int m_uY1;
		unsigned int m_uX2;
		unsigned int m_uY2;
		unsigned int m_uChannel;
		int m_nThreshold;
	};

	HoughNode() {}
	HoughNode(const VotingData & _votingData);      // create non-leaf node
	HoughNode(std::unique_ptr<HoughLeaf> & _leaf);  // create leaf node

	void showLeaf() const;
	bool isLeaf() const {return m_leaf.operator bool();}
	const VotingData & getVotingData() const {return m_voting;}
	const HoughLeaf * getLeaf() const {return m_leaf.get();}

	void save(std::ofstream & _stream) const;
	void load(std::ifstream & _stream);

private:
	VotingData m_voting;
	std::unique_ptr<HoughLeaf> m_leaf; // null if not leaf
};

#endif // HoughNode_H
