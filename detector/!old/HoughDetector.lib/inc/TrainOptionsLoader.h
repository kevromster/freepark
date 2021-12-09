#ifndef TrainOptionsLoader_H
#define TrainOptionsLoader_H

#include "HoughForestOptionsLoader.h"

#include "TrainOptions.h"

class TrainOptionsLoader : public HoughForestOptionsLoader
{
public:
	TrainOptionsLoader(const std::string & _strConfigFileName, TrainOptions & _options);

	virtual void loadAll();

protected:
	virtual void _initKnownOptions();
	virtual void _setOption(const std::string & _strOption, const std::string & _strValue);
	virtual void _checkAllOptionsSet() const;
	virtual bool _isKnownOption(const std::string & _strOption) const;

private:
	void _loadPositiveTrainFile();
	void _loadNegativeTrainFile();

	void _setTrainPositiveFiles(const std::string & _strOption) {m_options.setTrainPositiveFiles(_strOption);}
	void _setTrainPositivePath(const std::string & _strOption) {m_options.setTrainPositivePath(_strOption);}
	void _setTrainNegativeFiles(const std::string & _strOption) {m_options.setTrainNegativeFiles(_strOption);}
	void _setTrainNegativePath(const std::string & _strOption) {m_options.setTrainNegativePath(_strOption);}
	void _setTrainedForestPath(const std::string & _strOption) {m_options.setTrainedForestPath(_strOption);}

private:
	TrainOptions & m_options;
	HoughForestOptionsSetter<TrainOptionsLoader> m_optionsSetter;
};

#endif // TrainOptionsLoader_H
