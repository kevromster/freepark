#ifndef PatchStorage_H
#define PatchStorage_H

#include "ImagePatch.h"
#include "TrainImageInfo.h"

class PatchStorage
{
public:
	PatchStorage();

	void initialize(const cv::Size & _patchSize, unsigned int _uPatchesCount);

	cv::RNG & getRandomGenerator() const {return m_randomGenerator;}

	const std::vector<ImagePatch> & getPositivePatches() const {return m_positivePatches;}
	const std::vector<ImagePatch> & getNegativePatches() const {return m_negativePatches;}

	double getPositiveNegativeRatio() const {return double(m_positivePatches.size())/double(m_negativePatches.size());}

	// extract patches from training image
	void extractPositivePatches(const TrainImageInfo & _imageInfo);
	void extractNegativePatches(const std::string & _strImageFileName);

	static
	std::vector<std::unique_ptr<cv::Mat>> extractChannels(const cv::Mat & _image);

private:
	std::vector<ImagePatch> _extractPatches(
		const cv::Rect & _objectBBox,
		const cv::Point & _objectCenter,
		const std::vector<std::unique_ptr<cv::Mat>> & _imageChannels
	) const;

	void _initRandomGenerator();

private:
	unsigned int m_uPatchesCountToExtract;
	cv::Size m_patchSize;
	mutable cv::RNG m_randomGenerator;

	std::vector<ImagePatch> m_positivePatches;
	std::vector<ImagePatch> m_negativePatches;
};

#endif // PatchStorage_H
