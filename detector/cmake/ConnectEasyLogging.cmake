function(ConnectEasyLogging)
	set(EASYLOGGING_DIR "${THIRDPARTY_DIR}/easylogging")

	# using SYSTEM allows the compiler to think this is third-party code, so it doesn't show any warnings from there
	include_directories(SYSTEM ${EASYLOGGING_DIR})

	# need to add this source as compilation unit for easylogging connection
	set(EASYLOGGING_SRC "${EASYLOGGING_DIR}/easylogging++.cc" PARENT_SCOPE)
endfunction(ConnectEasyLogging)
