project(HoughDetector)

ConnectEasyLogging()
ConnectOpenCVLibs(${HDLIB_OPENCV_REQUIRED})
CopyCommonLibs()

set(HoughDetector_SOURCES
	src/main.cpp
	src/HDApp.cpp
	inc/HDApp.h
	inc/HDAppLog.h
)

include_directories(
	${PROJECT_SOURCE_DIR}/../HoughDetector.lib/pub
	${PROJECT_SOURCE_DIR}/inc
)

add_definitions(
	-DHD_DLL
)

add_executable(${PROJECT_NAME} ${HoughDetector_SOURCES} ${EASYLOGGING_SRC})
target_link_libraries(${PROJECT_NAME} ${HDLIB_NAME})

if (WIN32)
	add_custom_command(
		TARGET ${PROJECT_NAME}
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy "${HDLIB_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}${HDLIB_NAME}${CMAKE_SHARED_LIBRARY_SUFFIX}" ${PROJECT_BINARY_DIR})
endif()
