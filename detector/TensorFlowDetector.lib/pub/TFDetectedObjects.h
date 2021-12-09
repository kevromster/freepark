#ifndef TFDetectedObjects_H
#define TFDetectedObjects_H

#include "TFDetectedObject.h"

#include <functional>

/**
 * Represents information about all objects detected on an image.
 */
class TF_API TFDetectedObjects
{
public:
	TFDetectedObjects(std::vector<TFDetectedObject> && _detectedObjects) :
		m_detectedObjects(_detectedObjects)
	{}

	const std::vector<TFDetectedObject> & getDetectedObjects() const {return m_detectedObjects;}

	/**
	 * Sorts stored array of detected object via the given comparison function.
	 *
	 * @param compareFunc the comparison function to sort by
	 */
	template<typename T>
	void sortDetectedObjects(std::function<T> compareFunc) {
		std::sort(m_detectedObjects.begin(), m_detectedObjects.end(), compareFunc);
	}

private:
	std::vector<TFDetectedObject> m_detectedObjects;
};

#endif // TFDetectedObjects_H
