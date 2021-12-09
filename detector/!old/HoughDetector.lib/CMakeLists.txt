project(HoughDetector.lib)

set(HDLIB_NAME hddetect)

ConnectOpenCVLibs(core imgproc highgui imgcodecs)
ConnectEasyLogging()

set(HoughDetectorLib_SOURCES
	src/HDDetector.cpp
	src/HoughForest.cpp
	src/HoughTree.cpp
	src/HoughTreeTrainer.cpp
	src/HoughLeaf.cpp
	src/HoughNode.cpp
	src/TrainOptions.cpp
	src/DetectOptions.cpp
	src/HoughForestOptionsLoader.cpp
	src/TrainOptionsLoader.cpp
	src/DetectOptionsLoader.cpp
	src/PatchStorage.cpp
	src/Utils.cpp
	src/HDLibLog.cpp
	src/HDTrainer.cpp
	inc/HoughLeaf.h
	inc/HoughNode.h
	inc/HoughTree.h
	inc/HoughTreeTrainer.h
	inc/HoughForest.h
	inc/HoughForestOptions.h
	inc/TrainOptions.h
	inc/DetectOptions.h
	inc/ImagePatch.h
	inc/PatchStorage.h
	inc/TrainImageInfo.h
	inc/HoughForestOptionsLoader.h
	inc/HoughForestOptionsSetter.h
	inc/TrainOptionsLoader.h
	inc/DetectOptionsLoader.h
	inc/Utils.h
	inc/HDDetector.h
	inc/HDTrainer.h
	inc/HDLibLog.h
	pub/HDLog.h
	pub/HDExport.h
	pub/HDException.h
	pub/HDDetectedObject.h
	pub/HDDetectedObjects.h
	pub/IHDDetector.h
	pub/IHDTrainer.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/inc
	${PROJECT_SOURCE_DIR}/pub
)

add_definitions(
	-DHD_DLL
	-DHD_DLL_EXPORTS
)

if(CMAKE_BUILD_TYPE MATCHES [dD][eE][bB][uU][gG])
	add_definitions(-DSHOW_INTERMEDIATE_IMAGES)
endif()

add_library(${HDLIB_NAME} SHARED ${HoughDetectorLib_SOURCES} ${EASYLOGGING_SRC})
target_link_libraries(${HDLIB_NAME} ${OpenCV_LIBS})

set(HDLIB_NAME ${HDLIB_NAME} PARENT_SCOPE)
set(HDLIB_DIR ${PROJECT_BINARY_DIR} PARENT_SCOPE)
set(HDLIB_OPENCV_REQUIRED ${OpenCV_LIBS} PARENT_SCOPE)
