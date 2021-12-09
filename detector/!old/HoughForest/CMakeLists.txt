project(HoughForest)

ConnectOpenCVLibs(core imgproc highgui ml legacy)
CopyCommonLibs()

set(HoughForest_SOURCES
	CRForest-Detector.cpp
	CRForestDetector.cpp
	CRForestDetector.h
	CRForest.h
	CRPatch.cpp
	CRPatch.h
	CRTree.cpp
	CRTree.h
	HoG.cpp
	HoG.h
)

include_directories(
	${OPENCV_INCLUDE_DIR}/opencv
)

add_executable(${PROJECT_NAME} ${HoughForest_SOURCES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
