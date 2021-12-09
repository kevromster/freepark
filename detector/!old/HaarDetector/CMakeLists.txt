project(HaarDetector)

ConnectOpenCVLibs(core imgproc highgui objdetect)
CopyCommonLibs()

set(HaarDetector_SOURCES
	main.cpp
)

set(HaarDetector_test_pic test_cars1.jpg)
set(HaarDetector_cascade cars3.xml)

add_executable(${PROJECT_NAME} ${HaarDetector_SOURCES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

file(COPY ${HaarDetector_test_pic} DESTINATION ${PROJECT_BINARY_DIR})
file(INSTALL ${HaarDetector_cascade} DESTINATION ${PROJECT_BINARY_DIR})
