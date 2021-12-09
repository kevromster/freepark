#include "TFApp.h"
#include "TFException.h"

#include <iostream>

namespace {

const int g_nResultSuccess = 0;
const int g_nResultError = 1;

int usage() {
	std::cerr << "Usage: TensorFlowDetector <graph-file> <input-image-file> <output-file-name>" << std::endl;
	return g_nResultError;
}

int detect(const std::string & _strGraphFile, const std::string & _strImageFile, const std::string & _strOutFile) {
	TFApp app(_strGraphFile);
	app.detect(_strImageFile, _strOutFile);
	return g_nResultSuccess;
}

} // anonymous namespace

int main(int argc, char** argv) {
	try {
		std::cout << "Welcome to TensorFlowDetector app." << std::endl;

		if (argc != 4)
			return usage();

		const std::string strGraphFile = argv[1];
		const std::string strImageFile = argv[2];
		const std::string strOutputFile = argv[3];

		return detect(strGraphFile, strImageFile, strOutputFile);
	} catch (TFException & _ex) {
		std::cerr << "TFException caught: " << _ex.message() << std::endl;
		return g_nResultError;
	} catch (const std::exception & _ex) {
		std::cerr << "std::exception caught: " << _ex.what() << std::endl;
		return g_nResultError;
	} catch (...) {
		std::cerr << "Unknown exception caught! Exit from app.." << std::endl;
		return g_nResultError;
	}
}
