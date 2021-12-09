#include "TrainOptionsLoader.h"
#include "HDException.h"
#include "HDLibLog.h"
#include "Utils.h"

#include <fstream>

TrainOptionsLoader::TrainOptionsLoader(const std::string & _strConfigFileName, TrainOptions & _options) :
	HoughForestOptionsLoader(_strConfigFileName, _options),
	m_options(_options)
{
}

void TrainOptionsLoader::_initKnownOptions() {
	HoughForestOptionsLoader::_initKnownOptions();
	m_optionsSetter.addKnownOption("train_positive_files", &TrainOptionsLoader::_setTrainPositiveFiles);
	m_optionsSetter.addKnownOption("train_positive_path", &TrainOptionsLoader::_setTrainPositivePath);
	m_optionsSetter.addKnownOption("train_negative_files", &TrainOptionsLoader::_setTrainNegativeFiles);
	m_optionsSetter.addKnownOption("train_negative_path", &TrainOptionsLoader::_setTrainNegativePath);
}

void TrainOptionsLoader::_setOption(const std::string & _strOption, const std::string & _strValue) {
	if (m_optionsSetter.isKnownOption(_strOption))
		m_optionsSetter.setOption(*this, _strOption, _strValue);
	else
		HoughForestOptionsLoader::_setOption(_strOption, _strValue);
}

bool TrainOptionsLoader::_isKnownOption(const std::string & _strOption) const {
	return m_optionsSetter.isKnownOption(_strOption) || HoughForestOptionsLoader::_isKnownOption(_strOption);
}

void TrainOptionsLoader::_checkAllOptionsSet() const {
	m_optionsSetter.checkAllOptionsSet();
	HoughForestOptionsLoader::_checkAllOptionsSet();
}

void TrainOptionsLoader::loadAll() {
	HoughForestOptionsLoader::loadAll();
	_loadNegativeTrainFile();
	_loadPositiveTrainFile();
}

void TrainOptionsLoader::_loadPositiveTrainFile() {
	m_options.clearPositiveTrainImages();

	if (m_options.getTrainPositiveFiles().empty() || m_options.getTrainPositivePath().empty())
		throw ParseOptionsException("bad positive files config file name or path");

	std::ifstream in(m_options.getTrainPositiveFiles());

	if (!in.is_open())
		throw ParseOptionsException("Positive files config file not found: " + m_options.getTrainPositiveFiles());

	const int SZ = 1024;
	char buffer[SZ];
	std::string line;
	std::vector<std::string> vSplittedLine;

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		in.getline(buffer, SZ);

		if (in.bad())
			throw ParseOptionsException("Error reading stream of positive files config");

		line = buffer;
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		enum {
			FILE_NAME = 0,
			BBOX_LEFT,
			BBOX_TOP,
			BBOX_WIDTH,
			BBOX_HEIGHT,
			BBOX_CENTER_X,
			BBOX_CENTER_Y,
			OPTS_COUNT
		};

		vSplittedLine = split(line, ' ');

		if (vSplittedLine.size() != OPTS_COUNT) {
			LOG(WARNING) << "Wrong option at line " << uLine << ", ignore it";
			continue;
		}

		TrainImageInfo info;
		info.m_strFileName = m_options.getTrainPositivePath() + "/" + trim(vSplittedLine[FILE_NAME]);
		std::istringstream(trim(vSplittedLine[BBOX_CENTER_X])) >> info.m_center.x;
		std::istringstream(trim(vSplittedLine[BBOX_CENTER_Y])) >> info.m_center.y;
		std::istringstream(trim(vSplittedLine[BBOX_LEFT])) >> info.m_bbox.x;
		std::istringstream(trim(vSplittedLine[BBOX_TOP])) >> info.m_bbox.y;
		std::istringstream(trim(vSplittedLine[BBOX_WIDTH])) >> info.m_bbox.width;
		std::istringstream(trim(vSplittedLine[BBOX_HEIGHT])) >> info.m_bbox.height;

		if(info.m_bbox.width < m_options.getPatchSize().width || info.m_bbox.height < m_options.getPatchSize().height) {
			LOG(WARNING) << "Width or height are too small at line " << uLine << ", ignore it";
			continue;
		}

		if (info.m_bbox.width != m_options.getTrainObjectSize().width || info.m_bbox.height != m_options.getTrainObjectSize().height) {
			LOG(WARNING)
				<< "Bad bbox parameters at line " << uLine << ": expected ("
				<< m_options.getTrainObjectSize().width << "," << m_options.getTrainObjectSize().height << "), got ("
				<< info.m_bbox.width << "," << info.m_bbox.height << "), ignore it";
			continue;
		}

		m_options.addPositiveTrainImage(std::move(info));
	}

	if (m_options.getPositiveTrainImages().size() == 0)
		throw ParseOptionsException("No any positive files extracted from config file");
}

void TrainOptionsLoader::_loadNegativeTrainFile() {
	m_options.clearNegativeTrainImages();

	if (m_options.getTrainNegativeFiles().empty() || m_options.getTrainNegativePath().empty())
		throw ParseOptionsException("bad negative files config file name or path");

	std::ifstream in(m_options.getTrainNegativeFiles());

	if (!in.is_open())
		throw ParseOptionsException("Negative files config file not found: " + m_options.getTrainNegativeFiles());

	const int SZ = 1024;
	char buffer[SZ];

	for (unsigned int uLine = 1; in.good(); ++uLine) {
		in.getline(buffer, SZ);

		if (in.bad())
			throw ParseOptionsException("Error reading stream of negative files config");

		std::string line = buffer;
		trim(line);

		if (line.empty() || line.front() == '#')
			continue;

		m_options.addNegativeTrainImage(m_options.getTrainNegativePath() + "/" + line);
	}

	if (m_options.getNegativeTrainImages().size() == 0)
		throw ParseOptionsException("No any negative files extracted from config file");
}
