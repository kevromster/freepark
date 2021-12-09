function(ConnectYandexDiskLibs)
	set(YADISK_LIB "/usr/local")
	set(YADISK_LIB_NAME "yadisk-client")
	include_directories(
		"${YADISK_LIB}/include"
		"${THIRDPARTY_DIR}/nlohmann-json"
	)
	set(YandexDisk_LIBS "${YADISK_LIB}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${YADISK_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" PARENT_SCOPE)
endfunction(ConnectYandexDiskLibs)
