cmake_minimum_required(VERSION 2.8)

project(Traffic_Limitation)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIR}
)

if(APPLE)
	include_directories(
		${QT_INCLUDE_DIR}/../../src/network/access
		${QT_INCLUDE_DIR}/../../include/QtCore
	)
endif(APPLE)

if(WIN32)
	include_directories(
		${QT_INCLUDE_DIR}/../src/network/access
	)
endif(WIN32)

if(DEVELOPER_QT5)
	include_directories(
		${Qt5Network_PRIVATE_INCLUDE_DIRS}
		${Qt5Core_PRIVATE_INCLUDE_DIRS}
	)
endif(DEVELOPER_QT5)

set(HEADERS_TOMOC
	InterceptTimer.h
	NetworkReplyAdaptor.h
)

set(HEADERS
	InterceptingApplication.h
	qplatformdefs.h
	TrafficControl.h
	QConnectionObjectEx.h
)

set(SOURCES
	NetworkReplyAdaptor.cpp
	TrafficControl.cpp
	InterceptTimer.cpp
)

source_group("sources" FILES 
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

add_library(Traffic_Limitation STATIC
	${HEADERS_TOMOC} 
	${HEADERS} 
	${SOURCES}
	${MOC_FILES}
)

add_dependencies(Traffic_Limitation
	QtSingleApplication
	Utilities
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "common")
