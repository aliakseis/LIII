if(APPLE)
	
	write_file(${CMAKE_CURRENT_BINARY_DIR}/src/bundle_config.cmake "") # Stub for file
	macro(APPEND_DYNAMIC_LIB LIBRARY)
		get_filename_component(LIBRARY_PATH ${LIBRARY} PATH)
		get_filename_component(LIBRARY_NAME ${LIBRARY} NAME)

		get_target_property(projLocation ${PROJECT_NAME} LOCATION)
		string(REPLACE "/Contents/MacOS/${PROJECT_NAME}" "" MACOSX_BUNDLE_LOCATION ${projLocation})
		string(REPLACE "$(CONFIGURATION)$(EFFECTIVE_PLATFORM_NAME)" "$(CONFIGURATION)" APPS ${MACOSX_BUNDLE_LOCATION})

		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND cp \"${LIBRARY}\" \"${APPS}/Contents/MacOS/\")
		add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND install_name_tool -change \"${LIBRARY}\" \"@executable_path/../MacOS/${LIBRARY_NAME}\" \"${projLocation}\")
		set(BUNDLE_LIBRARIES_MOVE "${BUNDLE_LIBRARIES_MOVE};${APPS}/Contents/MacOS/${LIBRARY_NAME}")
		set(BUNDLE_DIRECTORY_MOVE "${BUNDLE_DIRECTORY_MOVE};${LIBRARY_PATH}")
		# override fix_bundle config
		write_file(${CMAKE_CURRENT_BINARY_DIR}/bundle_config.cmake "
			set(BUNDLE_LIBRARIES_MOVE \"${BUNDLE_LIBRARIES_MOVE}\")
			set(BUNDLE_DIRECTORY_MOVE \"${BUNDLE_DIRECTORY_MOVE}\")
		")
	endmacro(APPEND_DYNAMIC_LIB)
endif(APPLE)