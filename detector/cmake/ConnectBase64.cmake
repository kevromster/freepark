function(ConnectBase64)
	set(BASE64_DIR "${THIRDPARTY_DIR}/base64")

	# using SYSTEM allows the compiler to think this is third-party code, so it doesn't show any warnings from there
	include_directories(SYSTEM ${BASE64_DIR})

	# need to add this source as compilation unit for base64 connection
	set(BASE64_SRC "${BASE64_DIR}/base64.cpp" PARENT_SCOPE)
endfunction(ConnectBase64)
