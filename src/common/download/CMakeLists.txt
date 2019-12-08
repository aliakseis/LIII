cmake_minimum_required(VERSION 2.8)

project(Download)

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

set(HEADERS_DETAIL
	detail/downloader_base.h
	detail/downloader_signal_catchers.h
)

source_group("sources\\detail" FILES
	${HEADERS_TOMOC_DETAIL}
	${HEADERS_DETAIL}
)

set(HEADERS
	downloader.h
	downloader_traits.h
)

set(SOURCES
	detail/downloader_signal_catchers.cpp
)

source_group(sources FILES
	${HEADERS}
)

QTX_WRAP_CPP(MOC_FILES
	${HEADERS_TOMOC_DETAIL} 
)

source_group(".generated" FILES
	${MOC_FILES} 
)


add_library(Download STATIC
	${HEADERS_DETAIL}
	${HEADERS}
	${SOURCES}
)	

add_dependencies(Download
	Traffic_Limitation
	Utilities
)

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "common")
