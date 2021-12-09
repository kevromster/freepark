function(ConnectTgBotLibs)
	set(TGBOT_LIB "/usr/local")
	set(TGBOT_LIB_NAME "TgBot")
	include_directories(
		"${TGBOT_LIB}/include"
	)
	set(TgBot_LIBS "${TGBOT_LIB}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${TGBOT_LIB_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}" PARENT_SCOPE)
endfunction(ConnectTgBotLibs)
