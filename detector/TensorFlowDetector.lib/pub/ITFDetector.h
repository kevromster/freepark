#ifndef ITFDetector_H
#define ITFDetector_H

#include "TFDetectedObjects.h"

#include <opencv2/core.hpp>

#include <memory>

/**
 * Objects detector based on TensorFlow.
 */
class TF_API ITFDetector
{
public:
	virtual ~ITFDetector() {}

	/**
	 * Creates new detector instance.
	 *
	 * @param _strGraphFile file name with serialized TensorFlow's Graph object
	 * @return the detector instance
	 */
	static std::unique_ptr<ITFDetector> createDetector(const std::string & _strGraphFile);

	/**
	 * Runs detection alrogithm on the specified image given as a file name.
	 *
	 * @param _strInputImageFile input image file name
	 * @param _dDetectionThreshold detection threshold in interval (0;1], set 0.5 if not sure
	 * @param _classIdsToDetect vector of class Ids to be detected, can be empty to detect all known classes
	 * @return the vector of regions with detected objects in the coordinates of the image
	 */
	virtual std::unique_ptr<TFDetectedObjects> detectObjects(
		const std::string & _strInputImageFile,
		double _dDetectionThreshold,
		const std::vector<long> & _classIdsToDetect
	) = 0;

	/**
	 * Runs detection alrogithm on the specified image.
	 *
	 * @param _inputImage input image
	 * @param _dDetectionThreshold detection threshold in interval (0;1], set 0.5 if not sure
	 * @param _classIdsToDetect vector of class Ids to be detected, can be empty to detect all known classes
	 * @return the vector of regions with detected objects in the coordinates of the image
	 */
	virtual std::unique_ptr<TFDetectedObjects> detectObjects(
		const cv::Mat & _inputImage,
		double _dDetectionThreshold,
		const std::vector<long> & _classIdsToDetect
	) = 0;
};

#endif // ITFDetector_H
