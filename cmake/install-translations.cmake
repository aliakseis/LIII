
macro(INSTALL_TRANSLATIONS PROJECT_NAME)

	set(QM_FILES ${ARGV})
	list(REMOVE_ITEM QM_FILES ${PROJECT_NAME})

	if(WIN32)
		install(
			FILES		${QM_FILES}
			DESTINATION ./Translations
		)
		add_custom_command(
			TARGET ${PROJECT_NAME}
			COMMAND
				${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:${PROJECT_NAME}>/Translations
		)
		foreach(qmFile ${QM_FILES})
			add_custom_command(
				TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND
					${CMAKE_COMMAND} -E copy \"${qmFile}\" $<TARGET_FILE_DIR:${PROJECT_NAME}>/Translations
			)
		endforeach(qmFile ${QM_FILES})
	endif(WIN32)

	if(APPLE)
		get_target_property(projLocation ${PROJECT_NAME} LOCATION)
		string(REPLACE "/Contents/MacOS/${PROJECT_NAME}" "" MACOSX_BUNDLE_LOCATION ${projLocation})
		set(maxosxTranslatinosLocation
			${MACOSX_BUNDLE_LOCATION}/Contents/Resources/Translations
		)

		add_custom_command(TARGET ${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E make_directory ${maxosxTranslatinosLocation})

		foreach(FILENAME ${QM_FILES})
			string(REGEX MATCH "[.]*(_[a-z]+).qm$" lang ${FILENAME})
			set(targetFilename "translations${lang}")
			add_custom_command(TARGET ${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -E copy \"${FILENAME}\" ${maxosxTranslatinosLocation}/${targetFilename})
			# Instalation not needed... it will be done by bundle
		endforeach(FILENAME)
	elseif(UNIX)
		install(
			FILES ${QM_FILES}
			DESTINATION ${CLIENT_DATA_DIR}/translations
		)
	endif(APPLE)

endmacro(INSTALL_TRANSLATIONS)
