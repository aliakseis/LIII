# Macro thats setups Doxygen subproject
macro(SETUP_DOXYGEN)
	find_package(Doxygen QUIET)
	if(DOXYGEN_FOUND)
		configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
		add_custom_target(
			Doxygen
			${DOXYGEN_EXECUTABLE} 
			${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
			WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
			COMMENT "Generating API documentation with Doxygen" VERBATIM
		)
		if(WIN32)
			add_custom_command(TARGET Doxygen PRE_BUILD COMMAND if not exist \"${CMAKE_CURRENT_SOURCE_DIR}/doc/html\" ${CMAKE_COMMAND} -E copy_directory ${QT_BINARY_DIR}/../doc/html ${CMAKE_CURRENT_SOURCE_DIR}/doc/html)
		endif(WIN32)
	endif(DOXYGEN_FOUND)
endmacro(SETUP_DOXYGEN)
