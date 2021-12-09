project(CannyDetector)

ConnectOpenCVLibs(core imgproc highgui)
CopyCommonLibs()

set(CannyDetector_SOURCES
	main.cpp
	ImageWindow.h
	ImageWindow.cpp
	CannyApplier.h
	CannyApplier.cpp
	CannyWindow.h
	CannyWindow.cpp
)

set(CannyDetector_test_pic test_pic.jpg)

add_executable(${PROJECT_NAME} ${CannyDetector_SOURCES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

file(COPY ${CannyDetector_test_pic} DESTINATION ${PROJECT_BINARY_DIR})
