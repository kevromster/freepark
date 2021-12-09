#ifndef TrainOptions_H
#define TrainOptions_H

#include "HoughForestOptions.h"
#include "TrainImageInfo.h"

#include <vector>

/**
 * Options for training HoughForest algorithm.
 */
class TrainOptions : public HoughForestOptions
{
public:
	static TrainOptions createFromFile(const std::string & _strConfigFile);

	bool areCorrect() const {
		return
			HoughForestOptions::areCorrect() &&
			!m_strTrainPositiveFiles.empty() &&
			!m_strTrainPositivePath.empty() &&
			!m_strTrainNegativeFiles.empty() &&
			!m_strTrainNegativePath.empty();
	}

	const std::string & getTrainPositiveFiles() const {return m_strTrainPositiveFiles;}
	const std::string & getTrainPositivePath() const {return m_strTrainPositivePath;}

	const std::string & getTrainNegativeFiles() const {return m_strTrainNegativeFiles;}
	const std::string & getTrainNegativePath() const {return m_strTrainNegativePath;}

	const std::vector<TrainImageInfo> & getPositiveTrainImages() const {return m_positiveTrainImages;}
	const std::vector<std::string> & getNegativeTrainImages() const {return m_negativeTrainImages;}

	void setTrainPositiveFiles(const std::string & _file) {m_strTrainPositiveFiles = _file;}
	void setTrainPositivePath(const std::string & _path) {m_strTrainPositivePath = _path;}

	void setTrainNegativeFiles(const std::string & _file) {m_strTrainNegativeFiles = _file;}
	void setTrainNegativePath(const std::string & _path) {m_strTrainNegativePath = _path;}

	void clearPositiveTrainImages() {m_positiveTrainImages.clear();}
	void clearNegativeTrainImages() {m_negativeTrainImages.clear();}

	void addPositiveTrainImage(TrainImageInfo && _imageInfo) {m_positiveTrainImages.push_back(_imageInfo);}
	void addNegativeTrainImage(std::string && _imageInfo) {m_negativeTrainImages.push_back(_imageInfo);}

private:
	std::string m_strTrainPositiveFiles;
	std::string m_strTrainPositivePath;

	std::string m_strTrainNegativeFiles;
	std::string m_strTrainNegativePath;

	std::vector<TrainImageInfo> m_positiveTrainImages;
	std::vector<std::string> m_negativeTrainImages;
};

#endif // TrainOptions_H
