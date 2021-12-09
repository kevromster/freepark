function(CopyCommonLibs)
	# currently have job here only for WIN platform
	set(THIRDPARTY_COMMON_DIR "${THIRDPARTY_DIR}/common")
	if (WIN32)
		set(THIRDPARTY_LIBS_DIR "${THIRDPARTY_COMMON_DIR}/lib/win64")
		file(GLOB THIRDPARTY_LIBS RELATIVE ${PROJECT_SOURCE_DIR} "${THIRDPARTY_LIBS_DIR}/*")
		file(INSTALL ${THIRDPARTY_LIBS} DESTINATION ${PROJECT_BINARY_DIR})
	endif()
endfunction(CopyCommonLibs)
