#ifndef ImagePatch_H
#define ImagePatch_H

#include <array>
#include <memory>

#include <opencv2/core.hpp>

enum PATCH_CHANNELS {
	PATCH_CHANNEL_INTENCITY = 0,
	PATCH_CHANNEL_IX,
	PATCH_CHANNEL_IY,

	PATCH_CHANNELS_COUNT
};

// vector from one point to another
class PointsVector
{
public:
	PointsVector() {}
	PointsVector(const cv::Point & _ptFrom, const cv::Point & _ptTo) :
		m_ptFrom(_ptFrom), m_ptTo(_ptTo)
	{}

	const cv::Point getVector() const {return m_ptTo - m_ptFrom;}

private:
	cv::Point m_ptFrom;
	cv::Point m_ptTo;
};

class ImagePatch
{
public:
	void setBBox(const cv::Rect & _bbox) {m_bbox = _bbox;}
	void setCenter(const PointsVector & _patchCenter) {m_center = _patchCenter;}

	const cv::Rect & getBBox() const {return m_bbox;}
	const PointsVector & getCenterVector() const {return m_center;}

	void addPatch(unsigned int _uChannelIdx, const cv::Mat & _patch) {
		assert(_uChannelIdx < PATCH_CHANNELS_COUNT);
		m_channelPatches[_uChannelIdx] = std::make_unique<cv::Mat>(_patch.clone());
	}

	const cv::Mat & getPatch(unsigned int _uChannelIdx) const {
		assert(_uChannelIdx < PATCH_CHANNELS_COUNT);
		assert(m_channelPatches[_uChannelIdx]);
		return *m_channelPatches[_uChannelIdx];
	}

private:
	// bounding box and vector from object to patch center on the original image
	cv::Rect m_bbox;
	PointsVector m_center;

	// extracted patches for each channel
	std::array<std::unique_ptr<cv::Mat>, PATCH_CHANNELS_COUNT> m_channelPatches;
};

#endif // ImagePatch_H
