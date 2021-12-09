project(freepark_old_tryings)
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

set(WS_ROOT_DIR "${PROJECT_SOURCE_DIR}/..")
set(THIRDPARTY_DIR "${WS_ROOT_DIR}/3rdparty")

add_compile_options(
	-Wall
	-std=c++14
	-fvisibility=hidden
	-fvisibility-inlines-hidden
)

include(../cmake/CopyCommonLibs.cmake)
include(../cmake/ConnectOpenCVLibs.cmake)
include(../cmake/ConnectTensorFlowLibs.cmake)
include(../cmake/ConnectEasyLogging.cmake)
include(../cmake/ConnectBase64.cmake)

add_subdirectory(CannyDetector)
add_subdirectory(HaarDetector)
add_subdirectory(HoughForest)
add_subdirectory(HoughDetector.lib)
add_subdirectory(HoughDetector)
