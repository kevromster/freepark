#ifndef IHDTrainer_H
#define IHDTrainer_H

#include "HDExport.h"

#include <memory>

/**
 * Trainer of a Hough forest.
 * Generates Hough forest on the base of input training images.
 */
class HD_API IHDTrainer
{
public:
	virtual ~IHDTrainer() {}

	/**
	 * Creates new trainer instance.
	 *
	 * @param _strConfigFile file name with configuration options
	 * @return the trainer instance
	 */
	static std::unique_ptr<IHDTrainer> createTrainer(const std::string & _strConfigFile);

	/**
	 * Runs HoughForest training algorithm on the base of training images and configuration options
	 * described in the context's configuration file.
	 */
	virtual void train() = 0;

	/**
	 * Shows leafs of the trained Hough forest.
	 * Uses OpenCV's cv::imshow() function to display images.
	 * The method can be used to debug training alrogithm.
	 */
	virtual void showTrainedLeafs() = 0;
};

#endif // IHDTrainer_H
