project(TakeScreenshots)

ConnectOpenCVLibs(core highgui videoio imgcodecs)
ConnectYandexDiskLibs()
CopyCommonLibs()

# required by Yandex.Disk
find_package(Boost REQUIRED COMPONENTS system filesystem)

set(TakeScreenshots_SOURCES
	src/TakeScreenshots.cpp
)

add_executable(${PROJECT_NAME} ${TakeScreenshots_SOURCES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} ${YandexDisk_LIBS} ${Boost_LIBRARIES} -lcurl)
