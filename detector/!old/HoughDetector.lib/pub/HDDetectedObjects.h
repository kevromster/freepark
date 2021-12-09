#ifndef HDDetectedObjects_H
#define HDDetectedObjects_H

#include "HDDetectedObject.h"

/**
 * Represents information about all objects detected on an image.
 */
class HD_API HDDetectedObjects
{
public:
	HDDetectedObjects(
		std::vector<HDDetectedObject> && _detectedObjects,
		std::vector<cv::Mat> && _houghImages
	) :
		m_detectedObjects(_detectedObjects),
		m_houghImages(_houghImages)
	{}

	const std::vector<HDDetectedObject> & getDetectedObjects() const {return m_detectedObjects;}
	const std::vector<cv::Mat> & getHoughImages() const {return m_houghImages;}

	typedef bool (*CompareFunc)(const HDDetectedObject & _o1, const HDDetectedObject & _o2);

	void sortDetectedObjects(CompareFunc compareFunc) {
		std::sort(m_detectedObjects.begin(), m_detectedObjects.end(), compareFunc);
	}

private:
	std::vector<HDDetectedObject> m_detectedObjects;
	const std::vector<cv::Mat> m_houghImages;
};

#endif // HDDetectedObjects_H
