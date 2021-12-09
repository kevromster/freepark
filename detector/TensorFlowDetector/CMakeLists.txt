project(TensorFlowDetector)

CopyCommonLibs()
ConnectTensorFlowLibs()
ConnectOpenCVLibs(${TFLIB_OPENCV_REQUIRED} imgproc)

set(TensorFlowDetector_SOURCES
	src/main.cpp
	src/TFApp.cpp
	inc/TFApp.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/../TensorFlowDetector.lib/pub
	${PROJECT_SOURCE_DIR}/inc
)

add_definitions(
	-DTF_DLL
)

add_executable(${PROJECT_NAME} ${TensorFlowDetector_SOURCES})
target_link_libraries(${PROJECT_NAME} ${TFLIB_NAME})

if (WIN32)
	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${TFLIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${TFLIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}" ${PROJECT_BINARY_DIR})
endif()
