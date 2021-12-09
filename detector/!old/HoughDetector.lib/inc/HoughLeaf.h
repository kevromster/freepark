#ifndef HoughLeaf_H
#define HoughLeaf_H

#include <vector>

#include <opencv2/core.hpp>

class HoughLeaf
{
public:
	HoughLeaf();
	HoughLeaf(double _dForegroundProbability, size_t _sizeOfPatchCenters);

	void addPatchCenter(const cv::Point & _pt);
	void showPatchCenters() const;

	double getForegroundProbability() const {return m_dForegroundProbability;}
	const std::vector<cv::Point> & getPatchCenters() const {return m_vPatchCenters;}

	void save(std::ofstream & _stream) const;
	void load(std::ifstream & _stream);

private:
	double m_dForegroundProbability;

	// vectors from object center to training patches
	std::vector<cv::Point> m_vPatchCenters;
};

#endif // HoughLeaf_H
