cmake_minimum_required(VERSION 2.8)

if(POLICY CMP0020)
	cmake_policy(SET CMP0020 NEW)
endif(POLICY CMP0020)


###################################################
###  SECTION Paths
###################################################

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/main
	${CMAKE_CURRENT_SOURCE_DIR}/gui
	${CMAKE_CURRENT_SOURCE_DIR}/logic
	${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
	${CMAKE_CURRENT_SOURCE_DIR}/common
	${CMAKE_CURRENT_BINARY_DIR}/main
	${CMAKE_CURRENT_BINARY_DIR}/gui
	${CMAKE_CURRENT_BINARY_DIR}/logic
	${CMAKE_CURRENT_BINARY_DIR}/3rdparty
	${CMAKE_CURRENT_BINARY_DIR}/common
	${CMAKE_CURRENT_BINARY_DIR}
	${ZLIB_INCLUDE_DIR}
)

###################################################
###  END OF SECTION Paths
###################################################


###################################################
###  SECTION Modules 
###################################################

add_subdirectory(3rdparty/qtsingleapplication)

	find_package(TorrentRasterbar REQUIRED)
	include_directories(${TORRENT_INCLUDE_DIR})

if(APPLE)
	add_subdirectory(common/darwin)
	set(DARWIN_LIB Darwin)
endif(APPLE)

add_subdirectory(common/utilities)
add_subdirectory(common/download)
add_subdirectory(common/ui_utils)

if(DEVELOPER_TRAFFIC_CONTROL)
	add_subdirectory(common/traffic_limitation)
endif(DEVELOPER_TRAFFIC_CONTROL)


###################################################
###  END OF SECTION Modules 
###################################################

set(FUNCTIONAL_MODULE
	${PROJECT_NAME}_LIB CACHE STRING "Module with functional of the program"
)

####################################################
###  SECTION Sources
###################################################

##############
# Main group
##############

set(HEADERS_MAIN
	main/global_functions.h
	main/globals.h
	main/settings_declaration.h
)

set(HEADERS_MAIN_TOMOC
	main/application.h
)

set(SOURCES_MAIN
	main/application.cpp
	main/global_functions.cpp
)

source_group(main FILES
	${HEADERS_MAIN}
	${HEADERS_MAIN_TOMOC}
	${SOURCES_MAIN}
	${RESOURCE_OS}
)


##############
# GUI group
##############

set(HEADERS_TOMOC_GUI
	gui/buttonlabel.h
	gui/add_links.h
	gui/add_links_widget.h
	gui/LIIIstyle.h
	gui/lineeditex.h
	gui/logindialog.h
	gui/mainwindow.h
	gui/preferences.h
	gui/proplistdelegate.h
	gui/downloadcollectiondelegate.h
)

set(SOURCES_GUI
	gui/buttonlabel.cpp
	gui/add_links.cpp
	gui/add_links_widget.cpp
	gui/LIIIstyle.cpp
	gui/lineeditex.cpp
	gui/logindialog.cpp
	gui/mainwindow.cpp
	gui/preferences.cpp
	gui/proplistdelegate.cpp
	gui/downloadcollectiondelegate.cpp
)

set(FORMS_GUI
	gui/addtorrentform.ui
	gui/add_links.ui
	gui/logindialog.ui
	gui/mainwindow.ui
	gui/preferences.ui
	gui/torrentdetailsform.ui
)

source_group("gui\\sources" FILES
	${HEADERS_TOMOC_GUI}
	${SOURCES_GUI}
)

source_group("gui\\forms" FILES
	${FORMS_GUI}
)


##############
# Logic group
##############

set(HEADERS_TOMOC_LOGIC
	logic/downloadtype.h
	logic/downloadmanager.h
	logic/downloadtask.h
	logic/downloadcollectionmodel.h
	logic/downloadcollectiontreeview.h
	logic/treeitem.h
)

set(HEADERS_LOGIC
	logic/commandlineparser.h
	logic/configurableproxyfactory.h
)

set(SOURCES_LOGIC
	logic/commandlineparser.cpp
	logic/configurableproxyfactory.cpp
	logic/downloadtype.cpp
	logic/downloadmanager.cpp
	logic/downloadtask.cpp
	logic/downloadcollectionmodel.cpp
	logic/downloadcollectiontreeview.cpp
	logic/treeitem.cpp
)

source_group(logic FILES
	${HEADERS_LOGIC}
	${HEADERS_TOMOC_LOGIC}
	${SOURCES_LOGIC}
)

	set(HEADERS_TOMOC_TORRENT
		logic/torrentslistener.h	
		logic/torrentmanager.h
		logic/torrentcontentmodel.h
		logic/torrentdetailscontentview.h
		logic/torrentcontentfiltermodel.h
		gui/addtorrentform.h
        gui/peersinfoform_proxy_model.h
		gui/torrentdetailsform.h
		logic/peersinfomodel.h
	)

	set(HEADERS_TORRENT
		logic/torrentcontentmodelitem.h
	)

	set(SOURCES_TORRENT
		logic/torrentmanager.cpp
		logic/torrentslistener.cpp
		logic/torrentcontentmodelitem.cpp
		logic/torrentcontentmodel.cpp
		logic/torrentdetailscontentview.cpp
		logic/torrentcontentfiltermodel.cpp
		gui/addtorrentform.cpp
        gui/peersinfoform_proxy_model.cpp
		gui/torrentdetailsform.cpp
		logic/peersinfomodel.cpp
	)
	
	source_group(torrent FILES
		${HEADERS_TORRENT}
		${HEADERS_TOMOC_TORRENT}
		${SOURCES_TORRENT}
	)
	
############
# Assembly
############

set(HEADERS_TOMOC
	${HEADERS_TOMOC_GUI}
	${HEADERS_TOMOC_LOGIC}
	${HEADERS_MAIN_TOMOC}
)

set(HEADERS
	${HEADERS_LOGIC}
	${HEADERS_MAIN}
)

set(SOURCES
	${SOURCES_GUI}
	${SOURCES_LOGIC}
	${SOURCES_MAIN}
)

set(FORMS
	${FORMS_GUI}
)

	set(HEADERS_TOMOC 
		${HEADERS_TOMOC}
		${HEADERS_TOMOC_TORRENT}
	)
	set(HEADERS 
		${HEADERS}
		${HEADERS_TORRENT}
	)
	set(SOURCES 
		${SOURCES}
		${SOURCES_TORRENT}
	)

###################################################
###  END OF SECTION Sources
###################################################


###################################################
###  SECTION Generation
###################################################

QTX_ADD_RESOURCES(QRC_RESOURCES
	${RESOURCE_QT}
)

QTX_WRAP_CPP(MOC_FILES
	${HEADERS_TOMOC}
)

QTX_WRAP_UI(UI_FILES
	${FORMS}
)

source_group(resources FILES
	${RESOURCE_QT_IN}
	${RESOURCE_OS_IN}
	${CSS_FILES}
)

source_group(".generated" FILES ${UI_FILES} ${MOC_FILES} ${QRC_RESOURCES})

###################################################
###  END OF SECTION Generation
###################################################


##################################################
###  SECTION Translations
###################################################

CREATE_BRANDING_HXX()

set(FILES_TO_TRANSLATE
	${FILES_TO_TRANSLATE}
	${CMAKE_CURRENT_SOURCE_DIR}/main/globals.h
	${SOURCES_GUI}
	${CMAKE_CURRENT_SOURCE_DIR}/main/application.cpp
	${UI_FILES}
	${SOURCES_STRATEGIES}
	${SOURCES_LOGIC}
	${SOURCES_TORRENT}
#	PARENT_SCOPE       -- do not write to parent scope, just read
)

#message(STATUS "FILES_TO_TRANSLATE=${FILES_TO_TRANSLATE}")

if(DEVELOPER_GENERATE_TRANSLATIONS)

	QTX_CREATE_TRANSLATION(QM_FILES 
		${TS_FILES}
		${FILES_TO_TRANSLATE}
		OPTIONS -no-obsolete
	)
	message("Generating translations")

else(DEVELOPER_GENERATE_TRANSLATIONS)

	QTX_ADD_TRANSLATION(QM_FILES 
		${TS_FILES}
	)

endif(DEVELOPER_GENERATE_TRANSLATIONS)

source_group(translations FILES
	${TS_FILES}
)
source_group("qm-files" FILES ${QM_FILES})

###################################################
###  END OF SECTION Translations
###################################################


###################################################
###  SECTION Build
###################################################

if(APPLE)
	SET_SOURCE_FILES_PROPERTIES(${QT_MACOSX_RESOURCES} PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources
	)
	SET_SOURCE_FILES_PROPERTIES(${QT_MACOSX_IMAGES} PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources/images
	)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_APPICON} PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources
	)
	SET_SOURCE_FILES_PROPERTIES(${MACOSX_UPDATER} PROPERTIES 
		MACOSX_PACKAGE_LOCATION Resources
	)
	set(RESOURCE_OS_ALL
		${QT_MACOSX_RESOURCES}
		${QT_MACOSX_IMAGES}
		${MACOSX_APPICON}
		${MACOSX_UPDATER}
	)
endif(APPLE)

set(PROJECT_LIBRARIES
	Utilities
	QtSingleApplication
	Download
	ui_utils
)

if(APPLE)
	set(PROJECT_LIBRARIES 
		${PROJECT_LIBRARIES} 
		Darwin
	)
endif()

if(DEVELOPER_TRAFFIC_CONTROL)
	set(PROJECT_LIBRARIES 
		${PROJECT_LIBRARIES}
		Traffic_Limitation
	)
endif(DEVELOPER_TRAFFIC_CONTROL)

	set(PROJECT_LIBRARIES 
		${PROJECT_LIBRARIES}
		${TORRENT_LIBRARIES}
	)

# Configure DESKTOP_TARGET variable
SET_DESKTOP_TARGET()


add_library(${FUNCTIONAL_MODULE} STATIC
	# Sources
	${HEADERS_TOMOC}
	${HEADERS}
	${SOURCES}

	# Genered resources
	${UI_FILES} 
	${MOC_FILES} 
	${QRC_RESOURCES}
	${QM_FILES}

	# Some source resources
	${TS_FILES}
	${CSS_FILES}
)

target_link_libraries(${FUNCTIONAL_MODULE}
	${QT_LIBRARIES}
	${EXTERNAL_LIBS}
	${PROJECT_LIBRARIES}
)

source_group(main FILES
	${RESOURCE_OS}
	main/main.cpp
)

add_executable(${PROJECT_NAME}
	${DESKTOP_TARGET}
	main/main.cpp

	# Genered resources
	${QRC_RESOURCES}
	${RESOURCE_OS}
	${RESOURCE_OS_ALL}
)

	set(QTX_QTMAIN_LIBRARY ${Qt5Core_QTMAIN_LIBRARIES})

target_link_libraries(${PROJECT_NAME}
	${QTX_QTMAIN_LIBRARY}
	${FUNCTIONAL_MODULE}
)

if(DEVELOPER_BUILD_TESTS)
	APPEND_COMMON_TESTS()
endif(DEVELOPER_BUILD_TESTS)


add_dependencies(${PROJECT_NAME}
	${PROJECT_LIBRARIES}
)

###################################################
###  END OF SECTION Build
###################################################


###################################################
###  Set product version
###################################################

VersionConf(${PROJECT_NAME} ${RESOURCE_OS_IN} ${RESOURCE_OS})

if(WIN32)
	string(REPLACE "." ";" versionList ${PROJECT_VERSION})
	list(GET versionList 0 PROJECT_VERSION_MAJOR)
	list(GET versionList 1 PROJECT_VERSION_MINOR1)
	set(CMAKE_EXE_LINKER_FLAGS "/VERSION:\"${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR1}\"" CACHE STRING "Resetting the default linker options." FORCE)
	string(REPLACE "." "," FILEVERSION ${PROJECT_VERSION})
	set(VERSION_STR ${PROJECT_VERSION})
elseif(APPLE)
	set(MACOSX_BUNDLE_EXECUTABLE "${PROJECT_NAME}")
	set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}")
	set(MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME})
	set(MACOSX_BUNDLE_COPYRIGHT "Copyright 2019 © ${PROJECT_COMPANYNAME} All Rights Reserved.")

	string(REPLACE "." ";" DOMAIN_LIST ${PROJECT_DOMAIN})
	list(REVERSE DOMAIN_LIST)
	list(GET DOMAIN_LIST 0 TLD)
	list(GET DOMAIN_LIST 1 SLD)
	set(MACOSX_BUNDLE_GUI_IDENTIFIER "${TLD}.${SLD}.${PROJECT_NAME}")
	
	if (${MACOSX_BUNDLE_VERSION})
		set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_FULLNAME} Version ${PROJECT_VERSION}.${MACOSX_BUNDLE_VERSION}")
		set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}.${MACOSX_BUNDLE_VERSION})
		set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}.${MACOSX_BUNDLE_VERSION})
	else(${MACOSX_BUNDLE_VERSION})
		set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_FULLNAME} Version ${PROJECT_VERSION}.0")
		set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}.0)
		set(MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}.0)
	endif()
	
	set_target_properties(${PROJECT_NAME} PROPERTIES 
		MACOSX_BUNDLE_ICON_FILE 
		${MACOSX_BUNDLE_ICON_FILE}
	)

	# Set a custom plist file for the app bundle
  	set_target_properties(${PROJECT_NAME} PROPERTIES 
  		MACOSX_BUNDLE_INFO_PLIST 
  		${CMAKE_SOURCE_DIR}/resources/MacOSXResources/app-Info.plist
  	)
endif()

###################################################
###  END Set product version
###################################################


###################################################
###  SECTION Post Build
###################################################

get_target_property(BINARY_FILE ${PROJECT_NAME} LOCATION)
get_filename_component(BINARY_PATH ${BINARY_FILE} PATH)


# Start to install
install(TARGETS ${PROJECT_NAME} DESTINATION ${BINARY_INSTALL_DIR})
INSTALL_TRANSLATIONS(${PROJECT_NAME} ${QM_FILES})

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND 
			mt -manifest "${CMAKE_SOURCE_DIR}/resources/win7.manifest" -outputresource:"${BINARY_FILE}"
		COMMAND
			if EXIST "${CMAKE_SOURCE_DIR}/sign/${PROJECT_FULLNAME}/Cert.pfx"(
			signtool sign /v /f "${CMAKE_SOURCE_DIR}/sign/${PROJECT_FULLNAME}/Cert.pfx" "${BINARY_FILE}")
	)
endif(WIN32)

# Copy apple bz2 lib
#if(APPLE)
#	APPEND_DYNAMIC_LIB(${BZIP2_LIB})
#endif(APPLE)

# Install Qt and SSL libs
if(DEVELOPER_BUILD_TESTS)
	set(QT_COMPONENTS_TO_USE ${QT_COMPONENTS_TO_USE} QTTEST)
endif(DEVELOPER_BUILD_TESTS)

INSTALL_QT(${PROJECT_NAME} ${QT_COMPONENTS_TO_USE})
INSTALL_QT_PLUGINS(${PROJECT_NAME})
INSTALL_OPENSSL(${PROJECT_NAME})
INSTALL_RUNTIME_LIBRARIES()


if(DEVELOPER_FEATURES)
	INSTALL_DEBUG_INFO_FILE()
endif(DEVELOPER_FEATURES)

TARGET_BUNDLEFIX(${PROJECT_NAME})


###################################################
###  END OF SECTION Post Build
###################################################
