cmake_minimum_required(VERSION 3.0)

if(POLICY CMP0020)
	cmake_policy(SET CMP0020 NEW)
endif(POLICY CMP0020)

if(POLICY CMP0048)
	cmake_policy(SET CMP0048 NEW)
endif(POLICY CMP0048)

###################################################
### SECTION Project Settings
###################################################

set(PROJECT_NAME
	"LIII" CACHE STRING "Project Name aka brand name"
)

set(PROJECT_FULLNAME
	"${PROJECT_NAME} BitTorrent Client" CACHE STRING "${PROJECT_NAME} fullname"
)

set(PROJECT_DOMAIN
	"${PROJECT_NAME}.com" CACHE STRING "Internet domain name for ${PROJECT_NAME}"
)

set(PROJECT_COMPANYNAME
	"Company name" CACHE STRING "Company name"
)

set(CMAKE_MODULE_PATH 
	${CMAKE_MODULE_PATH} 
	"${CMAKE_CURRENT_SOURCE_DIR}/cmake"
)

project(${PROJECT_NAME} VERSION 0.1.0.0)
message("Building ${PROJECT_NAME} version ${PROJECT_VERSION}")

###################################################
###  END OF SECTION Project Settings
###################################################


###################################################
###  SECTION Options
###################################################

option(DEVELOPER_BUILD_TESTS "Use Qt unit tests" OFF)
option(DEVELOPER_FEATURES "use some debug tricks for developers" OFF)
option(DEVELOPER_GENERATE_TRANSLATIONS "Generate ts-files for ${PROJECT_NAME}" OFF)
option(DEVELOPER_TRAFFIC_CONTROL "allow traffic control functionality" ON)
option(DEVELOPER_TORRENTS_LOGS "enable torrents logging" OFF)

###################################################
### END OF SECTION Options
###################################################


###################################################
###  SECTION Base build settings
###################################################

# Using pre-defined macros for this project
include(cmake/projecthelper.cmake)
include(cmake/testing.cmake)
include(cmake/doxigenHelper.cmake)
include(cmake/machelper.cmake)
include(cmake/additionalOptions.cmake)
include(cmake/VersionConf.cmake)
include(cmake/integrate-qt.cmake)
include(cmake/install-translations.cmake)

if(WIN32 OR APPLE)
	set(CMAKE_INSTALL_PREFIX 
		"${CMAKE_BINARY_DIR}/bin"
	)
endif(WIN32 OR APPLE)

if (APPLE)
	set(CLIENT_DATA_DIR "${PROJECT_NAME}.app")
	set(BINARY_INSTALL_DIR .)
elseif (WIN32)
	set (CLIENT_DATA_DIR ".")
	set(BINARY_INSTALL_DIR .)
else ()
	set (CLIENT_DATA_DIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}")
	set(BINARY_INSTALL_DIR bin)
endif()

# Setup testing utility for project
SETUP_TESTING()

###################################################
### END OF SECTION Base build settings
###################################################


###################################################
###  SECTION Macro Definitions
###################################################

DEFINE_DEFAULT_DEFINITIONS()

add_definitions(
	-DRARDLL 
	-DSILENT # for internal code
	-DPROJECT_CLIENT_DATA_DIR="${CLIENT_DATA_DIR}"
)

if(DEVELOPER_TRAFFIC_CONTROL)
  add_definitions(
	-DALLOW_TRAFFIC_CONTROL
)
endif(DEVELOPER_TRAFFIC_CONTROL)



###################################################
###  END OF SECTION Macro Definitions
###################################################


###################################################
###  SECTION Resources
###################################################

################
# Qt resources
################

set(RESOURCE_QT 
	${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}/resources-${PROJECT_NAME}.qrc
)

set(RESOURCE_QT_IN 	
	${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc.in
)

configure_file(${RESOURCE_QT_IN} ${RESOURCE_QT} @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/resources/qt.conf ${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}/qt.conf) # Qt plugins support



###################################################
###  SECTION Frameworks and Libraries
###################################################

######
# Qt
######



#DETECT_QT()

	set(QT_COMPONENTS_TO_USE 
		Qt5Core
		Qt5Gui
		Qt5Widgets
		Qt5Network
		Qt5Xml
		Qt5LinguistTools #?
	)

INTEGRATE_QT(${QT_COMPONENTS_TO_USE})

###########
# OpenSSL
###########

find_package (OpenSSL REQUIRED)


###################
# Doxygen
###################

SETUP_DOXYGEN()

###################
# Using externals
###################

include_directories(
	${OPENSSL_INCLUDE_DIR}
	${PYTHON_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}			
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_BINARY_DIR}/src
)

set(EXTERNAL_LIBS
	${QT_QGIF_LIBRARY} 
	${QT_QICO_LIBRARY}	
	${PYTHON_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${IMAGE_LIBS}
)

if(NOT WIN32)
	set(EXTERNAL_LIBS
		${EXTERNAL_LIBS}
		${ZLIB_LIBRARIES}
	)
endif(NOT WIN32)

if(WIN32)
elseif(APPLE)
	find_library(MACOSX_FRAMEWORK_COCOA Cocoa)
	find_library(MACOSX_FRAMEWORK_QTKIT QTKit)
	find_library(MACOSX_FRAMEWORK_APPKIT AppKit)
	
	set(MACOSX_FRAMEWORK_LIBS
		"-lz"
		${MACOSX_FRAMEWORK_COCOA}
		${MACOSX_FRAMEWORK_QTKIT}
	)

	set(EXTERNAL_LIBS
		${EXTERNAL_LIBS}
		${MACOSX_FRAMEWORK_LIBS}
	)
endif()

###################################################
###  END OF SECTION Frameworks and Libraries
###################################################



################
# OS resources
################

if(WIN32)
	set(RESOURCE_OS		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/winres.rc"			)
	set(ICON_FILE		"${CMAKE_SOURCE_DIR}/resources/${PROJECT_NAME}/images/icon.ico"	)
	set(RESOURCE_OS_IN	"${CMAKE_CURRENT_SOURCE_DIR}/resources/winres.rc.in"			)
elseif(APPLE)
	set(MACOSX_BUNDLE_ICON_FILE 
		appicon.icns
	)
	set(QT_MACOSX_RESOURCES
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/macresources.qrc
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/qt_menu.nib
	)
	set(QT_MACOSX_IMAGES
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/images/copyarrowcursor.png
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/images/forbiddencursor.png
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/images/leopard-unified-toolbar-on.png
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/images/pluscursor.png
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/images/spincursor.png
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/images/waitcursor.png
	)
	set(MACOSX_APPICON
		${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}/images/appicon.icns
	)
	set(MACOSX_UPDATER
		${CMAKE_CURRENT_SOURCE_DIR}/resources/MacOSXResources/updater/updaterRO.scpt
	)
	
	set(RESOURCE_OS		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/LIII.desktop")
	set(RESOURCE_OS_IN	"${CMAKE_CURRENT_SOURCE_DIR}/resources/LIII.desktop.in"	)
	set(ICON_FILE		"${MACOSX_APPICON}"											)
elseif(UNIX)
	set(RESOURCE_OS		"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/LIII.desktop")
	set(RESOURCE_OS_IN	"${CMAKE_CURRENT_SOURCE_DIR}/resources/LIII.desktop.in"	)
endif()

##########
# Styles
##########

set(CSS_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}/style.css
)

if (APPLE)
	set(CSS_FILES ${CSS_FILES}
		${CMAKE_CURRENT_SOURCE_DIR}/resources/${PROJECT_NAME}/macstyle.css
	)
endif (APPLE)

###################################################
###  END OF SECTION Resources
###################################################



###################################################
###  SECTION Languages Definitions
###################################################

set(TS_FILES
	${CMAKE_SOURCE_DIR}/translations/translations_en.ts
)

set(FILES_TO_TRANSLATE
	
)

###################################################
###  END OF SECTION Languages Definitions
###################################################


set_property( GLOBAL PROPERTY USE_FOLDERS ON)
add_subdirectory(src)
