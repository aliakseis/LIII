cmake_minimum_required(VERSION 2.8)

project(QtSingleApplication)

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
)

set(HEADERS_TOMOC_
	qtlocalpeer.h
	qtsingleapplication.h
	qtsinglecoreapplication.h
)

set(HEADERS_
	qtlockedfile.h
	QtLockedFile
	QtSingleApplication
)

set(SOURCES_
	qtlocalpeer.cpp
	qtlockedfile.cpp
	qtsingleapplication.cpp
	qtsinglecoreapplication.cpp
)

if(UNIX)
	set(SOURCES_
		${SOURCES_}
		qtlockedfile_unix.cpp
	)
endif(UNIX)

source_group("sources" FILES
	${HEADERS_TOMOC_}
	${HEADERS_}
	${SOURCES_}
)


QTX_WRAP_CPP(MOC_FILES_
	${HEADERS_TOMOC_} 
)

source_group(".generated" FILES
	${MOC_FILES_}
)


add_library(QtSingleApplication STATIC
	${HEADERS_TOMOC_}
	${HEADERS_}
	${SOURCES_}
	${MOC_FILES_}
)	

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "common")

