project(TrainImageCropper)

ConnectOpenCVLibs(core highgui imgproc imgcodecs)
CopyCommonLibs()

set(TrainImageCropper_SOURCES
	src/cropper.cpp
	src/Parkings.cpp
	inc/Parkings.h
	inc/ParkingRectangle.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/inc
)

add_executable(${PROJECT_NAME} ${TrainImageCropper_SOURCES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
