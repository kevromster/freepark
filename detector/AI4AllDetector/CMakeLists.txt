project(AI4AllDetector)

ConnectBase64()
ConnectEasyLogging()
ConnectTgBotLibs()
ConnectOpenCVLibs(${TFLIB_OPENCV_REQUIRED} imgproc videoio)
CopyCommonLibs()

set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL)

#add_compile_options(
#	-pthread
#)

if (CURL_FOUND)
	include_directories(${CURL_INCLUDE_DIRS})
	add_definitions(-DHAVE_CURL)
endif()

set(AI4AllDetector_SOURCES
	src/main.cpp
	src/ServerCommunicator.cpp
	src/AI4AllDetector.cpp
	src/AI4AllDetectorApp.cpp
	inc/AI4AllDetectorApp.h
	inc/AI4AllDetectorLog.h
	inc/AI4AllDetectorException.h
	inc/AI4AllDetector.h
	inc/SubmitCameraItem.h
	inc/ServerCommunicator.h
	inc/NotificationType.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/../TensorFlowDetector.lib/pub
	${PROJECT_SOURCE_DIR}/inc
	${Boost_INCLUDE_DIRS}
	${OPENSSL_INCLUDE_DIR}
	${THIRDPARTY_DIR}/nlohmann-json
)

add_definitions(
	-DTF_DLL
)

add_executable(${PROJECT_NAME} ${AI4AllDetector_SOURCES} ${EASYLOGGING_SRC} ${BASE64_SRC})
target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} ${TFLIB_NAME} ${TgBot_LIBS} ${OpenCV_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})
