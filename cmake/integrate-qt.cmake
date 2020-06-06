macro(QTX_WRAP_CPP)
		QT5_WRAP_CPP(${ARGN})
endmacro(QTX_WRAP_CPP)

macro(QTX_GENERATE_MOC)
		QT5_GENERATE_MOC(${ARGN})
endmacro(QTX_GENERATE_MOC)

macro(QTX_ADD_TRANSLATION)
		QT5_ADD_TRANSLATION(${ARGN})
endmacro(QTX_ADD_TRANSLATION)

macro(QTX_CREATE_TRANSLATION)
		QT5_CREATE_TRANSLATION(${ARGN})
endmacro(QTX_CREATE_TRANSLATION)

macro(QTX_WRAP_UI)
		QT5_WRAP_UI(${ARGN})
endmacro(QTX_WRAP_UI)

macro(QTX_ADD_RESOURCES)
		QT5_ADD_RESOURCES(${ARGN})
endmacro(QTX_ADD_RESOURCES)


macro(INTEGRATE_QT)

	set(USE_QT_DYNAMIC ON)
	set(QT_COMPONENTS_TO_USE ${ARGV})
	
	if(DEVELOPER_BUILD_TESTS)
		set(QT_COMPONENTS_TO_USE ${QT_COMPONENTS_TO_USE} Qt5Test)
	endif(DEVELOPER_BUILD_TESTS)	
	
	foreach(qtComponent ${QT_COMPONENTS_TO_USE})
		if(NOT ${qtComponent} STREQUAL "Qt5ScriptTools")
			find_package(${qtComponent} REQUIRED)
		else()
			find_package(${qtComponent} QUIET)
		endif()		
		
		include_directories( ${${qtComponent}_INCLUDE_DIRS} )		
		#STRING(REGEX REPLACE "Qt5" "" COMPONENT_SHORT_NAME ${qtComponent})		
		#set(QT_MODULES_TO_USE ${QT_MODULES_TO_USE} ${COMPONENT_SHORT_NAME})
		if(${${qtComponent}_FOUND} AND NOT(${qtComponent} STREQUAL "Qt5LinguistTools"))
			STRING(REGEX REPLACE "Qt5" "" componentShortName ${qtComponent})		
			set(QT_LIBRARIES ${QT_LIBRARIES} "Qt5::${componentShortName}")
		endif()
	
	endforeach(qtComponent ${QT_COMPONENTS_TO_USE})

	if(NOT Qt5ScriptTools_FOUND)
		add_definitions(-DQT_NO_SCRIPTTOOLS)
	endif(NOT Qt5ScriptTools_FOUND)

	if(DEVELOPER_OPENGL)
		find_package(Qt5OpenGL REQUIRED)
		include_directories(
			${Qt5OpenGL_INCLUDE_DIRS}
		)
	endif(DEVELOPER_OPENGL)

	SETUP_COMPILER_SETTINGS(${USE_QT_DYNAMIC})
	
endmacro(INTEGRATE_QT)


##############################################################################

macro(INSTALL_QT TARGET_NAME)

	set(DLIBS_TO_COPY_RELEASE "")
	set(DLIBS_TO_COPY_DEBUG "")

	list(FIND QT_COMPONENTS_TO_USE "Qt5Xml" QT_XML_INDEX) 
	if(NOT ${QT_XML_INDEX} EQUAL -1)
		get_target_property(libLocation ${Qt5Xml_LIBRARIES} LOCATION)
		string(REGEX REPLACE "Xml" "XmlPatterns" libLocation ${libLocation})
		QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${libLocation} "")
	endif()

	foreach(qtComponent ${QT_COMPONENTS_TO_USE} ${QT_DEBUG_COMPONENTS_TO_USE})
		if(NOT ${qtComponent} STREQUAL "Qt5LinguistTools")
			if(NOT "${${qtComponent}_LIBRARIES}" STREQUAL "")
			get_target_property(libLocation ${${qtComponent}_LIBRARIES} LOCATION)
			#message(${qtComponent} " location = " ${${qtComponent}_LIBRARIES})
			QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${libLocation} "")
			else(NOT "${${qtComponent}_LIBRARIES}" STREQUAL "")
				message("Canont find library ${qtComponent}_LIBRARIES")
			endif(NOT "${${qtComponent}_LIBRARIES}" STREQUAL "")
		endif()
	endforeach(qtComponent ${QT_COMPONENTS_TO_USE} ${QT_DEBUG_COMPONENTS_TO_USE})	

if(WIN32)
	if(NOT CMAKE_BUILD_TYPE)
		# Visual studio install
		foreach(buildconfig ${CMAKE_CONFIGURATION_TYPES})
			#message(STATUS "VC configuration install in INSTALL_QT")
			if(${buildconfig} STREQUAL "Debug")
				set(DLIBS_TO_COPY ${DLIBS_TO_COPY_ALL} ${DLIBS_TO_COPY_DEBUG})
			else()
				set(DLIBS_TO_COPY ${DLIBS_TO_COPY_ALL} ${DLIBS_TO_COPY_RELEASE})
			endif()

			install(FILES
				${DLIBS_TO_COPY}
				DESTINATION .
				CONFIGURATIONS ${buildconfig}
			)
		endforeach(buildconfig ${CMAKE_CONFIGURATION_TYPES})

	else(NOT CMAKE_BUILD_TYPE)
		# Make install
		message(STATUS "Make configuration install")
		if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
			set(DLIBS_TO_COPY ${DLIBS_TO_COPY_ALL} ${DLIBS_TO_COPY_DEBUG})
		else()
			set(DLIBS_TO_COPY ${DLIBS_TO_COPY_ALL} ${DLIBS_TO_COPY_RELEASE})
		endif()

		install(FILES
			${DLIBS_TO_COPY}
			DESTINATION .
		)
	endif(NOT CMAKE_BUILD_TYPE)
endif(WIN32)
		
endmacro(INSTALL_QT)

macro(ADD_TO_INSTALL_FILES TARGET_NAME copyToSubdirectory)
if(WIN32)
	if(NOT CMAKE_BUILD_TYPE)
		# Visual studio install
		message(STATUS "VC configuration install dir = " ${copyToSubdirectory})
		foreach(buildconfig ${CMAKE_CONFIGURATION_TYPES})
			if(${buildconfig} STREQUAL "Debug")
				set(DLIBS_TO_COPY ${DLIBS_TO_COPY_DEBUG})
			else()
				set(DLIBS_TO_COPY ${DLIBS_TO_COPY_RELEASE})
			endif()

			install(FILES
				${DLIBS_TO_COPY}
				DESTINATION ${copyToSubdirectory}
				CONFIGURATIONS ${buildconfig}
			)
		endforeach(buildconfig ${CMAKE_CONFIGURATION_TYPES})

	else(NOT CMAKE_BUILD_TYPE)
		# Make install
		message(STATUS "Make configuration install")
		if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
			set(DLIBS_TO_COPY ${DLIBS_TO_COPY_DEBUG})
		else()
			set(DLIBS_TO_COPY ${DLIBS_TO_COPY_RELEASE})
		endif()

		install(FILES
			${DLIBS_TO_COPY}
			DESTINATION ${copyToSubdirectory}
		)
	endif(NOT CMAKE_BUILD_TYPE)
endif(WIN32)
endmacro(ADD_TO_INSTALL_FILES)

macro(QT_ADD_POSTBUILD_STEP TARGET_NAME libLocation copyToSubdirectory)
	set(libLocation_release ${libLocation})

	#message(-------------QT_ADD_POSTBUILD_STEP--------------- ${libLocation} "; copy to " ${TARGET_NAME}${copyToSubdirectory})
	set(REPLACE_PATTERN "/lib/([^/]+)$" "/bin/\\1") # from lib to bin
	
	if(NOT EXISTS "${libLocation_release}")
		string(REGEX REPLACE ${REPLACE_PATTERN} libLocation_release ${libLocation_release})
		if(NOT EXISTS "${libLocation_release}")
			message(FATAL_ERROR "cannot add post_build step to ${libLocation_release}")
		endif()
	endif()	
	
	if(WIN32)
		string(REGEX REPLACE ".dll" "d.dll" libLocation_debug ${libLocation_release})
	elseif(APPLE)
		string(REGEX REPLACE ".dylib" "d.dylib" libLocation_debug ${libLocation_release})
	else()
		string(REGEX REPLACE ".so" "d.so" libLocation_debug ${libLocation_release})
	endif()

	set(DLIBS_TO_COPY_RELEASE ${DLIBS_TO_COPY_RELEASE} ${libLocation_release})
	set(DLIBS_TO_COPY_DEBUG ${DLIBS_TO_COPY_DEBUG} ${libLocation_debug})

	add_custom_command(TARGET ${TARGET_NAME} COMMAND
			${CMAKE_COMMAND} -E make_directory  $<TARGET_FILE_DIR:${TARGET_NAME}>${copyToSubdirectory}
			)

	add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND
		 ${CMAKE_COMMAND} -E copy $<$<CONFIG:Debug>:${libLocation_debug}> $<$<NOT:$<CONFIG:Debug>>:${libLocation_release}>  $<TARGET_FILE_DIR:${TARGET_NAME}>${copyToSubdirectory}
	)		
	#message(-------------------------------------------------)
endmacro(QT_ADD_POSTBUILD_STEP)


macro(INSTALL_IMAGEFORMATS_HELPER TYPE)
	if(${TYPE} STREQUAL "Debug")
		install(FILES
			${QT_${imgFormatPlugin}_PLUGIN_DEBUG}
			DESTINATION ${IMAGEFORMATS_DIR}
			CONFIGURATIONS ${TYPE}
			COMPONENT Runtime
		)
		string(REPLACE "${QT_IMAGEFORMATS_PLUGINS_DIR}" "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/MacOS/imageformats" QT_IMAGEFORMATS_PLUGIN_LOCAL ${QT_${imgFormatPlugin}_PLUGIN_DEBUG})
		list(APPEND BUNDLE_LIBRARIES_MOVE ${QT_IMAGEFORMATS_PLUGIN_LOCAL})
	else()
		install(FILES
				${QT_${imgFormatPlugin}_PLUGIN_RELEASE}
				DESTINATION ${IMAGEFORMATS_DIR}
				CONFIGURATIONS ${TYPE}
				COMPONENT Runtime
		)
		string(REPLACE "${QT_IMAGEFORMATS_PLUGINS_DIR}" "${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app/Contents/MacOS/imageformats" QT_IMAGEFORMATS_PLUGIN_LOCAL ${QT_${imgFormatPlugin}_PLUGIN_RELEASE})
		list(APPEND BUNDLE_LIBRARIES_MOVE ${QT_IMAGEFORMATS_PLUGIN_LOCAL})
	endif()
endmacro(INSTALL_IMAGEFORMATS_HELPER TYPE)

macro(INSTALL_IMAGEFORMATS TARGET_NAME)
	if(WIN32 OR APPLE)
		get_target_property(qtCoreLocation ${Qt5Core_LIBRARIES} LOCATION)
		string(REGEX REPLACE "(lib|bin)/Qt5Core(.*)" "plugins/imageformats" imageFormatsPath ${qtCoreLocation})
		string(REGEX REPLACE "(.*)(lib|bin)/Qt5Core." "" dllExtension ${qtCoreLocation})
		set(MyImageFormats qico qgif qjpeg)
		
		if(APPLE)
			get_target_property(PROJECT_LOCATION ${TARGET_NAME} LOCATION)
			string(REPLACE "/Contents/MacOS/${TARGET_NAME}" "" MACOSX_BUNDLE_LOCATION ${PROJECT_LOCATION})
			string(REPLACE "$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)" "$(CONFIGURATION)" BUNDLE_ROOT ${MACOSX_BUNDLE_LOCATION})

			set(IMAGEFORMATS_DIR ${BUNDLE_ROOT}/Contents/MacOS/imageformats)
		else(APPLE)
			set(IMAGEFORMATS_DIR imageformats)
			#message("IMAGEFORMATS_DIR = " ${IMAGEFORMATS_DIR})
		endif(APPLE)
		
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		foreach(imgFormatPlugin ${MyImageFormats})
			set(imagePlugin_release "${imageFormatsPath}/${imgFormatPlugin}.${dllExtension}")
			#message("imagePlugin_release = " ${imagePlugin_release})
			QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${imagePlugin_release} "/imageformats/")
		endforeach(imgFormatPlugin ${MyImageFormats})

		ADD_TO_INSTALL_FILES(${TARGET_NAME} ${IMAGEFORMATS_DIR})
	endif(WIN32 OR APPLE)
endmacro(INSTALL_IMAGEFORMATS TARGET_NAME)

macro(INSTALL_QT5PLUGINS TARGET_NAME)
	if(WIN32 OR APPLE)
		#message(--------------------------------------INSTALL_QT5PLUGINS----------------------------------)
		get_target_property(qtCoreLocation ${Qt5Core_LIBRARIES} LOCATION)
		string(REGEX REPLACE "(bin|lib)/Qt5Core(.*)" "plugins" qtPluginsPath ${qtCoreLocation})
		string(REGEX REPLACE "(.*)(bin|lib)/Qt5Core." "" dllExtension ${qtCoreLocation})
		
		####### PLATFORMS #######
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		set(platformPlugin_release "${qtPluginsPath}/platforms/qwindows.${dllExtension}")
		QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${platformPlugin_release} "/platforms/")

		ADD_TO_INSTALL_FILES(${TARGET_NAME} "platforms")
		#########################
		
		####### STYLES #######
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		if(WIN32)
			set(stylePlugin_release "${qtPluginsPath}/styles/qwindowsvistastyle.${dllExtension}")
		else(WIN32)
			set(stylePlugin_release "${qtPluginsPath}/styles/qmacstyle.${dllExtension}")
		endif(WIN32)
		QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${stylePlugin_release} "/styles/")

		ADD_TO_INSTALL_FILES(${TARGET_NAME} "styles")
		#########################
		
		####### accessible #######
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		set(accessiblePlugin_release "${qtPluginsPath}/accessible/qtaccessiblewidgets.${dllExtension}")

		ADD_TO_INSTALL_FILES(${TARGET_NAME} "accessible")
		#########################	
		
		######### MISC LIBS ONLY RELEASE ##########
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		string(REGEX REPLACE "(bin|lib)/Qt5Core(.*)" "bin" qtBinPath ${qtCoreLocation})
		set(MISC_LIBS icuuc49 icuin49 icudt49 icuuc51 icuin51 icudt51 D3DCompiler_43)
		foreach(miscLib ${MISC_LIBS})
			set(miscLib_release "${qtBinPath}/${miscLib}.${dllExtension}")
			if(EXISTS "${miscLib_release}")
				set(DLIBS_TO_COPY_RELEASE ${DLIBS_TO_COPY_RELEASE} ${miscLib_release})
				set(DLIBS_TO_COPY_DEBUG ${DLIBS_TO_COPY_DEBUG} ${miscLib_release})

				add_custom_command(TARGET ${TARGET_NAME} POST_BUILD COMMAND
					${CMAKE_COMMAND} -E copy $<$<CONFIG:Debug>:${miscLib_release}> $<$<NOT:$<CONFIG:Debug>>:${miscLib_release}>  $<TARGET_FILE_DIR:${TARGET_NAME}>
				)	
			endif()
		endforeach(miscLib ${MISC_LIBS})

		ADD_TO_INSTALL_FILES(${TARGET_NAME} ".")
		########################

		######### Qt5V8 ##########
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		set(ADDITION_LIBS Qt5V8)
		foreach(additionLib ${ADDITION_LIBS})
			set(additionLib_release "${qtBinPath}/${additionLib}.${dllExtension}")
			if(EXISTS "${additionLib_release}")
				#Smessage(------ADDITION_LIBS--${additionLib_release})
				QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${additionLib_release} "/")
			endif()
		endforeach(additionLib ${ADDITION_LIBS})

		ADD_TO_INSTALL_FILES(${TARGET_NAME} ".")
		##########################
		
		######## MISC LIBS DEBUG AND RELEASE #########
		set(DLIBS_TO_COPY_RELEASE "")
		set(DLIBS_TO_COPY_DEBUG "")

		set(MISC_LIBS libEGL libGLESv2)
		foreach(miscLib ${MISC_LIBS})
			set(miscLib_release "${qtBinPath}/${miscLib}.${dllExtension}")		
			if(EXISTS "${miscLib_release}")
				QT_ADD_POSTBUILD_STEP(${TARGET_NAME} ${miscLib_release} "")
			endif()
		endforeach(miscLib ${MISC_LIBS})

		ADD_TO_INSTALL_FILES(${TARGET_NAME} ".")
		###########################################
	endif(WIN32 OR APPLE)
endmacro(INSTALL_QT5PLUGINS TARGET_NAME)

macro(INSTALL_QT_PLUGINS TARGET_NAME)
	INSTALL_IMAGEFORMATS(${TARGET_NAME})
		INSTALL_QT5PLUGINS(${TARGET_NAME})
endmacro(INSTALL_QT_PLUGINS TARGET_NAME)
