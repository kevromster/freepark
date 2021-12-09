#include "HoughForestOptionsLoader.h"
#include "HoughForestOptions.h"
#include "HDException.h"
#include "HDLibLog.h"
#include "Utils.h"

#include <fstream>

HoughForestOptionsLoader::HoughForestOptionsLoader(const std::string & _strConfigFileName, HoughForestOptions & _options) :
	m_options(_options),
	m_strConfigFileName(_strConfigFileName)
{
}

void HoughForestOptionsLoader::loadAll() {
	_initKnownOptions();
	_loadConfigFile();
	_checkAllOptionsSet();

	if (!m_options.areCorrect())
		throw BadOptionException("Some options are INCORRECT after parsing!");
}

void HoughForestOptionsLoader::_initKnownOptions() {
	m_optionsSetter.addKnownOption("patch_width", &HoughForestOptionsLoader::_setPatchWidth);
	m_optionsSetter.addKnownOption("patch_height", &HoughForestOptionsLoader::_setPatchHeight);
	m_optionsSetter.addKnownOption("train_object_width", &HoughForestOptionsLoader::_setTrainObjectWidth);
	m_optionsSetter.addKnownOption("train_object_height", &HoughForestOptionsLoader::_setTrainObjectHeight);
	m_optionsSetter.addKnownOption("patches_count", &HoughForestOptionsLoader::_setPatchesCount);
	m_optionsSetter.addKnownOption("trees_count", &HoughForestOptionsLoader::_setTreesCount);
	m_optionsSetter.addKnownOption("trained_forest_path", &HoughForestOptionsLoader::_setTrainedForestPath);
	m_optionsSetter.addKnownOption("tree_max_depth", &HoughForestOptionsLoader::_setTreeMaxDepth);
	m_optionsSetter.addKnownOption("tree_min_patches_in_leaf", &HoughForestOptionsLoader::_setMinPatchesInLeaf);
	m_optionsSetter.addKnownOption("train_tree_iterations_count", &HoughForestOptionsLoader::_setTrainIterationsCount);
}

void HoughForestOptionsLoader::_setOption(const std::string & _strOption, const std::string & _strValue) {
	m_optionsSetter.setOption(*this, _strOption, _strValue);
}

bool HoughForestOptionsLoader::_isKnownOption(const std::string & _strOption) const {
	return m_optionsSetter.isKnownOption(_strOption);
}

void HoughForestOptionsLoader::_checkAllOptionsSet() const {
	m_optionsSetter.checkAllOptionsSet();
}

void HoughForestOptionsLoader::_loadConfigFile() {
	if (m_strConfigFileName.empty())
		throw ParseOptionsException("bad config file name");

	std::ifstream in(m_strConfigFileName);

	if (!in.is_open())
		throw ParseOptionsException("Main config file not found: " + m_strConfigFileName);

	const int SZ = 1024;
	char buffer[SZ];
	std::string line;
	std::vector<std::string> vSplittedLine;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		in.getline(buffer, SZ);

		if (in.bad())
			throw ParseOptionsException("Error reading stream of main config file");

		line = buffer;
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		vSplittedLine = split(line, '=');

		if (vSplittedLine.size() != 2) {
			LOG(WARNING) << "Wrong option description at line " << uLine << ", ignore it (acceptable format: <option>=<value>)";
			continue;
		}

		const std::string & strOption = trim(vSplittedLine[0]);
		const std::string & strValue = trim(vSplittedLine[1]);

		if (!_isKnownOption(strOption)) {
			LOG(WARNING) << "Unknown option \"" << strOption << "\" at line " << uLine << ", ignore it";
			continue;
		}

		// set value
		HDLOG << "Setting option \"" << strOption << "\" to \"" << strValue << "\"...";
		_setOption(strOption, strValue);
	}
}

void HoughForestOptionsLoader::_setTrainedForestPath(const std::string & _strOption) {
	m_options.setTrainedForestPath(_strOption);
}

void HoughForestOptionsLoader::_setPatchWidth(const std::string & _strOption) {
	int nWidth = 0;
	std::istringstream(_strOption) >> nWidth;
	if (nWidth > 0)
		m_options.setPatchWidth(nWidth);
}

void HoughForestOptionsLoader::_setPatchHeight(const std::string & _strOption) {
	int nHeight = 0;
	std::istringstream(_strOption) >> nHeight;
	if (nHeight > 0)
		m_options.setPatchHeight(nHeight);
}

void HoughForestOptionsLoader::_setTrainObjectWidth(const std::string & _strOption) {
	int nWidth = 0;
	std::istringstream(_strOption) >> nWidth;
	if (nWidth > 0)
		m_options.setTrainObjectWidth(nWidth);
}

void HoughForestOptionsLoader::_setTrainObjectHeight(const std::string & _strOption) {
	int nHeight = 0;
	std::istringstream(_strOption) >> nHeight;
	if (nHeight > 0)
		m_options.setTrainObjectHeight(nHeight);
}

void HoughForestOptionsLoader::_setPatchesCount(const std::string & _strOption) {
	unsigned int uCount = 0;
	std::istringstream(_strOption) >> uCount;
	if (uCount > 0)
		m_options.setPatchesCount(uCount);
}

void HoughForestOptionsLoader::_setTreesCount(const std::string & _strOption) {
	unsigned int uCount = 0;
	std::istringstream(_strOption) >> uCount;
	if (uCount > 0)
		m_options.setTreesCount(uCount);
}

void HoughForestOptionsLoader::_setTreeMaxDepth(const std::string & _strOption) {
	unsigned int uValue = 0;
	std::istringstream(_strOption) >> uValue;
	if (uValue > 0)
		m_options.setTreeMaxDepth(uValue);
}

void HoughForestOptionsLoader::_setMinPatchesInLeaf(const std::string & _strOption) {
	unsigned int uValue = 0;
	std::istringstream(_strOption) >> uValue;
	if (uValue > 0)
		m_options.setMinPatchesInLeaf(uValue);
}

void HoughForestOptionsLoader::_setTrainIterationsCount(const std::string & _strOption) {
	unsigned int uValue = 0;
	std::istringstream(_strOption) >> uValue;
	if (uValue > 0)
		m_options.setTrainIterationsCount(uValue);
}
