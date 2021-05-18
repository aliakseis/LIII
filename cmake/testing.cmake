macro(SETUP_TESTING)
	if(DEVELOPER_BUILD_TESTS)
		enable_testing()
		include(CTest)
	endif(DEVELOPER_BUILD_TESTS)
endmacro(SETUP_TESTING)

macro(APPEND_COMMON_TESTS)
	if(DEVELOPER_BUILD_TESTS)
	
		do_test(AuthenticationHelper "common/modules-tests/utilities/test-AuthenticationHelper.cpp" "common/modules-tests/utilities/test-AuthenticationHelper.h")
		do_test(Utils "common/modules-tests/utilities/test-Utils.cpp" "common/modules-tests/utilities/test-Utils.h")
		do_test(Downloader "common/modules-tests/download/test-Download.cpp" "common/modules-tests/download/test-Download.h")
		do_test(ui_utils "common/modules-tests/ui_utils/test-mainwindowwithtray.cpp" "common/modules-tests/ui_utils/test-mainwindowwithtray.h")
		do_test(resources_test "common/modules-tests/resources_test/test-resources.cpp" "common/modules-tests/resources_test/test-resources.h")
		
	endif(DEVELOPER_BUILD_TESTS)
endmacro(APPEND_COMMON_TESTS)

# Macro for tests
macro(do_test testname)
	#message(----------------BEGIN----------------)

	# clear variables
	set(RESOURCES_TEST)
	set(SOURCES_TEST)
	set(HDRS_TO_MOC)
	set(SRCS_TO_MOC)
	set(MOC_SRCS)
	set(MOC_HDRS)

	foreach(arg ${ARGN})
        #message("arg='${arg}'")
		string(REGEX MATCH "qrc_.+\\.cxx" RESOURCE ${arg})
		if(NOT "${RESOURCE}" STREQUAL "")
			SET(RESOURCES_TEST ${RESOURCES_TEST} ${RESOURCE})
		else()
			string(REGEX MATCH ".+\\.(cpp|h|cc|hh|hpp)" SRC ${arg})
			if(NOT "${SRC}" STREQUAL "")
				SET(SOURCES_TEST ${SOURCES_TEST} ${SRC})
			endif()	
		endif()
    endforeach()
	
	message("SOURCES_TEST= ${SOURCES_TEST}")
	
	# looking for files to moc
	foreach(SRC ${SOURCES_TEST})
		string(REGEX MATCH ".+\\.cpp" SRCMATCH ${SRC})
		if(NOT "${SRCMATCH}" STREQUAL "")
			string(REGEX REPLACE "(.+)(\\.cpp)" "\\1" SRCMATCH ${SRCMATCH})
			
			set(HAS_PAIR 0)
			foreach(SRC2 ${SOURCES_TEST})
				string(REGEX MATCH "${SRCMATCH}\\.h" HEADERMATCH ${SRC2})
				if(NOT "${HEADERMATCH}" STREQUAL "")
					#message("FOUNDPAIR: ${HEADERMATCH} ${SRC2}")
					set(HAS_PAIR 1)
				endif()
			endforeach()
		
			if(${HAS_PAIR})
				set(HDRS_TO_MOC ${HDRS_TO_MOC} "${SRCMATCH}.h")
			else()
				set(SRCS_TO_MOC ${SRCS_TO_MOC} "${SRCMATCH}.cpp")
			endif()
			#message(${SRCMATCH})
			
		endif()
	endforeach()
	
	foreach(TOMOC ${SRCS_TO_MOC})
		string(REGEX MATCH "[A-Za-z_\\-]+\\.cpp" TOMOC2 ${TOMOC})
		string(REGEX REPLACE "\\.cpp" ".${testname}.moc" TEST_MOC_FILE ${TOMOC2})
		set(MOC_${testname}_TEST "${CMAKE_CURRENT_BINARY_DIR}/${TEST_MOC_FILE}")
		#message("TOMOC2:${TOMOC2} ----- ${TEST_MOC_FILE} ")
		#message("TOMOC:${TOMOC} ----- ${MOC_${testname}_TEST}")
		QTX_GENERATE_MOC(${TOMOC} ${MOC_${testname}_TEST})
		SET_SOURCE_FILES_PROPERTIES(${TOMOC} PROPERTIES OBJECT_DEPENDS ${MOC_${testname}_TEST})
		set(MOC_SRCS ${MOC_SRCS} ${MOC_${testname}_TEST})
	endforeach()
	

	if(DEFINED HDRS_TO_MOC)
		QTX_WRAP_CPP(MOC_HDRS
			${HDRS_TO_MOC}
		)
	endif()

	
	#message(-----------------END-------------------)

	include_directories(${QT_QTTEST_INCLUDE_DIR})
	

	add_executable(test_${testname}
		${MOC_HDRS}
		${MOC_SRCS}
		${SOURCES_TEST}
		${RESOURCES_TEST}
	)

		set(QTX_TEST_LIBRARY ${Qt5Test_LIBRARIES})

	target_link_libraries(test_${testname}
		${FUNCTIONAL_MODULE}
		${QT_LIBRARIES}
		${QTX_TEST_LIBRARY}
		${OPENSSL_LIBRARIES}
	)

	set_property(TARGET test_${testname} PROPERTY FOLDER "Tests")

	ADD_TEST(test_${testname} test_${testname})
endmacro(do_test testname)
