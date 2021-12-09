#include "PatchStorage.h"
#include "HDException.h"
#include "HDLibLog.h"
#include "Utils.h"

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <ctime>

#ifdef SHOW_INTERMEDIATE_IMAGES

static
void _displayChannelImages(const std::vector<std::unique_ptr<cv::Mat>> & _channels) {
	int cnt = 0;
	for (const auto & image : _channels) {
		cv::imshow("channel_" + std::to_string(cnt++), *image);
		//cvSaveImage( buffer, vImg[i] );

		while (cv::waitKey() != 27);
	}

	for (int i = 0; i < cnt; ++i)
		cv::destroyWindow("channel_" + std::to_string(i));
}

static
void _displayImage(const cv::Mat & _image) {
	LOG(INFO) << "image size: " << _image.size().width << "x" << _image.size().height;
	cv::imshow("image", _image);
	while (cv::waitKey() != 27);
	cv::destroyWindow("image");
}

#endif // SHOW_INTERMEDIATE_IMAGES

PatchStorage::PatchStorage() :
	m_uPatchesCountToExtract(0)
{}

void PatchStorage::initialize(const cv::Size & _patchSize, unsigned int _uPatchesCount) {
	m_uPatchesCountToExtract = _uPatchesCount;
	m_patchSize = _patchSize;

	if (m_uPatchesCountToExtract == 0 || m_patchSize.width <= 0 || m_patchSize.height <= 0)
		throw BadOptionException();

	m_positivePatches.clear();
	m_negativePatches.clear();
	m_positivePatches.reserve(_uPatchesCount);
	m_negativePatches.reserve(_uPatchesCount);

	_initRandomGenerator();
}

void PatchStorage::_initRandomGenerator() {
	m_randomGenerator.state = std::time(nullptr);
}

void PatchStorage::extractPositivePatches(const TrainImageInfo & _imageInfo) {
	const cv::Mat image = cv::imread(_imageInfo.m_strFileName, cv::IMREAD_COLOR);
	if (!image.data)
		throw ImageReadException(_imageInfo.m_strFileName);

#ifdef SHOW_INTERMEDIATE_IMAGES
	//_displayImage(image);
#endif

	std::vector<ImagePatch> extractedPatches = _extractPatches(_imageInfo.m_bbox, _imageInfo.m_center, extractChannels(image));
	appendVector(m_positivePatches, extractedPatches);
}

void PatchStorage::extractNegativePatches(const std::string & _strImageFileName) {
	const cv::Mat image = cv::imread(_strImageFileName, cv::IMREAD_COLOR);
	if (!image.data)
		throw ImageReadException(_strImageFileName);

#ifdef SHOW_INTERMEDIATE_IMAGES
	//_displayImage(image);
#endif

	std::vector<ImagePatch> extractedPatches = _extractPatches(cv::Rect(cv::Point(0,0), image.size()), cv::Point(0,0), extractChannels(image));
	appendVector(m_negativePatches, extractedPatches);
}

std::vector<std::unique_ptr<cv::Mat>> PatchStorage::extractChannels(const cv::Mat & _image) {
	std::vector<std::unique_ptr<cv::Mat>> channels;
	channels.reserve(PATCH_CHANNELS_COUNT);

	for (unsigned int i = 0; i < PATCH_CHANNELS_COUNT; ++i)
		channels.push_back(std::make_unique<cv::Mat>(_image.size(), CV_8UC1));

	cv::cvtColor(_image, *channels[PATCH_CHANNEL_INTENCITY], cv::COLOR_BGR2GRAY);

	cv::Mat imageIx(_image.size(), CV_16SC1);
	cv::Mat imageIy(_image.size(), CV_16SC1);

	cv::Sobel(*channels[PATCH_CHANNEL_INTENCITY], imageIx, CV_16SC1, 1, 0, 3);
	cv::Sobel(*channels[PATCH_CHANNEL_INTENCITY], imageIy, CV_16SC1, 0, 1, 3);

	cv::convertScaleAbs(imageIx, *channels[PATCH_CHANNEL_IX], 0.25);
	cv::convertScaleAbs(imageIy, *channels[PATCH_CHANNEL_IY], 0.25);

#ifdef SHOW_INTERMEDIATE_IMAGES
	//_displayChannelImages(channels);
#endif

	return channels;
}

std::vector<ImagePatch> PatchStorage::_extractPatches(
	const cv::Rect & _objectBBox,
	const cv::Point & _objectCenter,
	const std::vector<std::unique_ptr<cv::Mat>> & _imageChannels
) const {
	assert(_imageChannels.size() == PATCH_CHANNELS_COUNT);
	std::vector<ImagePatch> extractedPatches;

	const int patchCenterX = m_patchSize.width/2;
	const int patchCenterY = m_patchSize.height/2;

	// generate x,y locations
	const cv::Mat locations(m_uPatchesCountToExtract, 1, CV_32SC2);
	m_randomGenerator.fill(
		locations,
		cv::RNG::UNIFORM,
		cv::Scalar(_objectBBox.x, _objectBBox.y),
		cv::Scalar(_objectBBox.x + _objectBBox.width - m_patchSize.width, _objectBBox.y + _objectBBox.height - m_patchSize.height)
	);

	extractedPatches.reserve(m_uPatchesCountToExtract);

	for (unsigned int i = 0; i < m_uPatchesCountToExtract; ++i) {
		const cv::Point & pt = locations.at<cv::Point>(i);

		extractedPatches.emplace_back();
		ImagePatch & patch = extractedPatches.back();
		patch.setBBox(cv::Rect(pt, m_patchSize));

		if (_objectCenter != cv::Point(0,0))
			patch.setCenter(PointsVector(_objectCenter, pt + cv::Point(patchCenterX, patchCenterY)));

		for (unsigned int uChannelIdx = 0; uChannelIdx < PATCH_CHANNELS_COUNT; ++uChannelIdx) {
			patch.addPatch(uChannelIdx, (*_imageChannels[uChannelIdx])(patch.getBBox()));

#ifdef SHOW_INTERMEDIATE_IMAGES
			//_displayImage(patch.getPatch(uChannelIdx));
#endif
		}
	}

	return extractedPatches;
}
