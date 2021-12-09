#ifndef IHDDetector_H
#define IHDDetector_H

#include "HDDetectedObjects.h"

#include <opencv2/core.hpp>

#include <memory>

/**
 * Objects detector on the base of HoughForest algorithm.
 */
class HD_API IHDDetector
{
public:
	virtual ~IHDDetector() {}

	/**
	 * Creates new detector instance.
	 *
	 * @param _strConfigFile file name with configuration options
	 * @return the detector instance
	 */
	static std::unique_ptr<IHDDetector> createDetector(const std::string & _strConfigFile);

	/**
	 * Runs detection alrogithm on the specified image given as file name.
	 *
	 * @param _strInputImageFile input image file name
	 * @return the vector of regions with detected objects in the coordinates of the image
	 */
	virtual std::unique_ptr<HDDetectedObjects> detectObjects(const std::string & _strInputImageFile) = 0;

	/**
	 * Runs detection alrogithm on the specified image.
	 *
	 * @param _inputImage input image
	 * @return the vector of regions with detected objects in the coordinates of the image
	 */
	virtual std::unique_ptr<HDDetectedObjects> detectObjects(const cv::Mat & _inputImage) = 0;
};

#endif // IHDDetector_H
