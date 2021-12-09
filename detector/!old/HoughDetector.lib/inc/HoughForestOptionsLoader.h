#ifndef HoughForestOptionsLoader_H
#define HoughForestOptionsLoader_H

#include "HoughForestOptionsSetter.h"

class HoughForestOptions;

/**
 * Loader of options for HoughForest algorithm from a configutaion file.
 */
class HoughForestOptionsLoader
{
public:
	HoughForestOptionsLoader(const std::string & _strConfigFileName, HoughForestOptions & _options);
	virtual ~HoughForestOptionsLoader() {}

	virtual void loadAll();

protected:
	virtual void _initKnownOptions();
	virtual void _setOption(const std::string & _strOption, const std::string & _strValue);
	virtual void _checkAllOptionsSet() const;
	virtual bool _isKnownOption(const std::string & _strOption) const;

private:
	void _loadConfigFile();
	void _loadPositiveTrainFile();
	void _loadNegativeTrainFile();

	void _setTrainedForestPath(const std::string & _strOption);
	void _setPatchWidth(const std::string & _strOption);
	void _setPatchHeight(const std::string & _strOption);
	void _setTrainObjectWidth(const std::string & _strOption);
	void _setTrainObjectHeight(const std::string & _strOption);
	void _setPatchesCount(const std::string & _strOption);
	void _setTreesCount(const std::string & _strOption);
	void _setTreeMaxDepth(const std::string & _strOption);
	void _setMinPatchesInLeaf(const std::string & _strOption);
	void _setTrainIterationsCount(const std::string & _strOption);

private:
	HoughForestOptions & m_options;
	std::string m_strConfigFileName;
	HoughForestOptionsSetter<HoughForestOptionsLoader> m_optionsSetter;
};

#endif // HoughForestOptionsLoader_H
