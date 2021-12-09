project(TensorFlowDetector.lib)

ConnectTensorFlowLibs()
ConnectOpenCVLibs(core highgui imgproc imgcodecs)

set(TFLIB_NAME tfdetect)

set(TFDetectorLib_SOURCES
	src/TFDetector.cpp
	inc/AutoDeleteWrapper.h
	inc/TFDetector.h
	pub/ITFDetector.h
	pub/TFExport.h
	pub/TFException.h
	pub/TFDetectedObject.h
	pub/TFDetectedObjects.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/pub
	${PROJECT_SOURCE_DIR}/inc
)

add_definitions(
	-DTF_DLL
	-DTF_DLL_EXPORTS
)

add_library(${TFLIB_NAME} SHARED ${TFDetectorLib_SOURCES})
target_link_libraries(${TFLIB_NAME} ${TensorFlow_LIBS} ${OpenCV_LIBS})

set(TFLIB_NAME ${TFLIB_NAME} PARENT_SCOPE)
set(TFLIB_DIR ${PROJECT_BINARY_DIR} PARENT_SCOPE)
set(TFLIB_OPENCV_REQUIRED ${OpenCV_LIBS} PARENT_SCOPE)
