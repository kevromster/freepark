project(CarDetector)

ConnectBase64()
ConnectEasyLogging()
ConnectOpenCVLibs(${TFLIB_OPENCV_REQUIRED} imgproc videoio)
CopyCommonLibs()

find_package(Boost REQUIRED COMPONENTS system filesystem)

add_compile_options(
	-pthread
)

set(CarDetector_SOURCES
	src/main.cpp
	src/Parking.cpp
	src/ParkingOptions.cpp
	src/MathUtils.cpp
	src/CarDetector.cpp
	src/CarDetectorApp.cpp
	src/GeoCoordinatePoint.cpp
	src/CameraOptions.cpp
	inc/CameraOptions.h
	inc/CarDetectorApp.h
	inc/CarDetectorLog.h
	inc/CarDetector.h
	inc/CarDetectorException.h
	inc/ParkingOptions.h
	inc/Parking.h
	inc/ParkingPlace.h
	inc/GeoCoordinatePoint.h
	inc/MathUtils.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/../TensorFlowDetector.lib/pub
	${PROJECT_SOURCE_DIR}/inc
	${Boost_INCLUDE_DIRS}
)

add_definitions(
	-DTF_DLL
)

if (WIN32)
	add_definitions(-D_USE_MATH_DEFINES)
endif()

if (CMAKE_BUILD_TYPE MATCHES [dD][eE][bB][uU][gG])
	add_definitions(-DSAVE_DEBUG_IMAGES)
endif()

add_executable(${PROJECT_NAME} ${CarDetector_SOURCES} ${EASYLOGGING_SRC} ${BASE64_SRC})
target_link_libraries(${PROJECT_NAME} ${TFLIB_NAME} ${OpenCV_LIBS} ${Boost_LIBRARIES} -lpthread)

if (WIN32)
	target_link_libraries(${PROJECT_NAME} -lws2_32)

	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${TFLIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${TFLIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}" ${PROJECT_BINARY_DIR})

	get_filename_component(BOOST_SYSTEM_LIBNAME "${Boost_SYSTEM_LIBRARY_RELEASE}" NAME)
	file(INSTALL "${Boost_LIBRARY_DIRS}/${BOOST_SYSTEM_LIBNAME}" DESTINATION ${PROJECT_BINARY_DIR})
endif()
