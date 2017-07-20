macro(VersionConf prjName from_file to_file)

set(COMPANYNAME "\"${PROJECT_COMPANYNAME}\"")
set(PRODUCTNAME "\"${PROJECT_FULLNAME}\"")


string(REPLACE "." ";" versionList ${PROJECT_VERSION})

list(LENGTH versionList versionLength)
if(${versionLength} LESS 3)
	message("Please set PROJECT_VERSION to 4 dot separated words, e.g. 1.0.1.124")
	set(versionList 0;0;0)
endif()

if(${versionLength} LESS 4)
	find_package(Subversion)
	if(Subversion_FOUND)
		# check if CMAKE_SOURCE_DIR is under svn
		execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} info "${CMAKE_SOURCE_DIR}" RESULT_VARIABLE Subversion_svn_info_result)
		if(${Subversion_svn_info_result} EQUAL 0)
			Subversion_WC_INFO(${CMAKE_SOURCE_DIR} SVNDATA)
			set(getRevFromSvn ON)
		else(${Subversion_svn_info_result} EQUAL 0)
			set(SVNDATA_WC_REVISION 0)
			set(getRevFromSvn OFF)
		endif(${Subversion_svn_info_result} EQUAL 0)
	else(Subversion_FOUND)
		set(SVNDATA_WC_REVISION 0)
		set(getRevFromSvn OFF)
	endif(Subversion_FOUND)
	set(versionList ${versionList} ${SVNDATA_WC_REVISION})
else()
	set(getRevFromSvn OFF)
endif()

# message("versionList=${versionList}; getRevFromSvn = ${getRevFromSvn}")

list(GET versionList 0 MAJOR_VER)
list(GET versionList 1 MINOR_VER1)
list(GET versionList 2 MINOR_VER2)
list(GET versionList 3 MINOR_VER3)

set(SVNDATA_WC_REVISION ${MINOR_VER3})

message("Subversion_FOUND=${Subversion_FOUND}")

set(filecontent "
	set(getRevFromSvn ${getRevFromSvn})
	if(getRevFromSvn)
		set(Subversion_SVN_EXECUTABLE ${Subversion_SVN_EXECUTABLE})
		find_package(Subversion)
	endif(getRevFromSvn)
	if(getRevFromSvn AND Subversion_FOUND)
		Subversion_WC_INFO(\"${CMAKE_SOURCE_DIR}\" SVNDATA)
	else(getRevFromSvn AND Subversion_FOUND)
		set(SVNDATA_WC_REVISION ${MINOR_VER3})
	endif(getRevFromSvn AND Subversion_FOUND)
	set(MAJOR_VER	${MAJOR_VER} )
	set(MINOR_VER1	${MINOR_VER1})
	set(MINOR_VER2	${MINOR_VER2})
	set(MINOR_VER3	\${SVNDATA_WC_REVISION})
	set(PRODUCTNAME ${PRODUCTNAME})
	set(SHORTPRODUCTNAME ${prjName})
	set(COMPANYNAME ${COMPANYNAME})
	set(PRODUCTDOMAIN ${PROJECT_DOMAIN})
	set(ICON_FILE     \"${ICON_FILE}\")
	set(ICON_FILE_16bit     \"${ICON_FILE_16bit}\")
	configure_file(\"${from_file}\" \"${to_file}\")
	file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/version.hxx\" \"#define PROJECT_VERSION \\\"\${MAJOR_VER}.\${MINOR_VER1}.\${MINOR_VER2}.\${MINOR_VER3}\\\"\\n\")
")

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/versionConfFile.cmake" "${filecontent}")

add_custom_target(VersionConf_${prjName} ALL DEPENDS ${to_file})

add_custom_command(OUTPUT ${to_file}
	COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_BINARY_DIR}/versionConfFile.cmake)

set_source_files_properties(
	${to_file} "${CMAKE_CURRENT_BINARY_DIR}/version.hxx"
	PROPERTIES GENERATED TRUE
)

source_group("main" FILES "${CMAKE_CURRENT_BINARY_DIR}/version.hxx")

add_dependencies(${FUNCTIONAL_MODULE} VersionConf_${prjName})
add_dependencies(${prjName} VersionConf_${prjName})

endmacro(VersionConf)
