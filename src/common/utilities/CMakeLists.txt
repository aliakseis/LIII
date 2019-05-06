cmake_minimum_required(VERSION 2.8)

project(Utilities)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(
	-DNOMINMAX 
	-D_CRT_SECURE_NO_WARNINGS 
	-D_CRT_NONSTDC_NO_WARNINGS 
	-DSILENT
)


set(HEADERS_TOMOC
	authentication_helper.h
	notify_helper.h
)

set(HEADERS
	authentication_helper.h
	autorun_utils.h
	credential.h
	credsretriever.h
	errorcode.h
	logger.h
	qttranslation.h
	singleton.h
	translation.h
	translatable.h
	utils.h
	notify_helper.h
	modelserializer.h
	modeldeserializer.h
	modelserializecommon.h
	iparentadvice.h
	instantiator.h
	patricia.h
	customutf8codec.h
	associate_app.h
	windowsfirewall.h
	filesaveguard.h
	filesystem_utils.h
)

set(SOURCES
	authentication_helper.cpp
	autorun_utils.cpp
	credsretriever.cpp
	errorcode.cpp
	logger.cpp
	translation.cpp
	translatable.cpp
	utils.cpp
	modelserializer.cpp
	modeldeserializer.cpp
	customutf8codec.cpp
	windowsfirewall.cpp
	filesystem_utils.cpp
)

if(WIN32)
	set(SOURCES
		${SOURCES} 	
		associate_app_win.cpp
	)
elseif(APPLE)
	set(SOURCES
		${SOURCES} 	
		associate_app_mac.cpp
	)
elseif(UNIX)
	set(SOURCES
		${SOURCES} 	
		associate_app_linux.cpp
	)
endif()

source_group(sources FILES
	${HEADERS_TOMOC}
	${HEADERS}
	${SOURCES}
)


QTX_WRAP_CPP(MOC_FILES
	${HEADERS_TOMOC}
)

source_group(".generated" FILES
	${MOC_FILES}
)

set(FILES_TO_TRANSLATE ${FILES_TO_TRANSLATE}
	${CMAKE_CURRENT_SOURCE_DIR}/qttranslation.h 
	${CMAKE_CURRENT_SOURCE_DIR}/errorcode.h
	${CMAKE_CURRENT_SOURCE_DIR}/errorcode.cpp
	PARENT_SCOPE
)

add_library(Utilities STATIC
	${HEADERS_TOMOC}
	${HEADERS}
	${SOURCES}
	${MOC_FILES}
)	

if(APPLE)
	add_dependencies(Utilities 
		Darwin
	)
endif(APPLE)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "common")
