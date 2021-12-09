function(CopyOpenCVLibs)
	# currently need custom copying only on WIN platform
	if (WIN32)
		set(VERSION_SUFFIX 2410)
		set(OpenCV_LIB_DIR ${OpenCV_DIR}/bin)

		foreach(lib ${OpenCV_LIBS})
			set(libfile "${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${VERSION_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX}")
			file(INSTALL "${OpenCV_LIB_DIR}/${libfile}" DESTINATION ${PROJECT_BINARY_DIR})
		endforeach()
	endif()
endfunction(CopyOpenCVLibs)

function(ConnectOpenCVLibs)
	find_package(OpenCV REQUIRED ${ARGV})
	include_directories(${OpenCV_INCLUDE_DIRS})
	CopyOpenCVLibs()
	set(OpenCV_LIBS ${OpenCV_LIBS} PARENT_SCOPE)
endfunction(ConnectOpenCVLibs)
