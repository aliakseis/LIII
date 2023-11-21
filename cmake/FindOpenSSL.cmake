# Find OpenSSL script

if(WIN32)

  unset(OPENSSL_ROOT_DIR CACHE)
  unset(OPENSSL_INCLUDE_DIR CACHE)

  set(CMAKE_MODULE_PATH_TEMP ${CMAKE_MODULE_PATH})
  unset(CMAKE_MODULE_PATH)
  find_package(OpenSSL)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH_TEMP})
  if (OPENSSL_FOUND)
	if (OPENSSL_ROOT_DIR)
		set(OPENSSL_BINARY_DIR ${OPENSSL_ROOT_DIR}/bin CACHE PATH "path to openSSL dlls" FORCE)
	else()
		set(OPENSSL_BINARY_DIR ${OPENSSL_INCLUDE_DIR}/../bin CACHE PATH "path to openSSL dlls" FORCE)
	endif()
	message( STATUS "Found openssl binary directory: ${OPENSSL_BINARY_DIR}" )
	message( STATUS "Found openssl include directory: ${OPENSSL_INCLUDE_DIR}" )
  else ()
	if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		message( STATUS "Searching 64bit openssl library" )
		set(PLATFORM_SUBDIR x64)
	else( CMAKE_SIZEOF_VOID_P EQUAL 8 )
		message( STATUS "Searching 32bit openssl library" )
		set(PLATFORM_SUBDIR x86)
	endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(OPENSSL_BINARY_DIR ${CMAKE_SOURCE_DIR}/imports/OpenSSL/${PLATFORM_SUBDIR}/bin CACHE PATH "path to openSSL dlls" FORCE)
	set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/imports/OpenSSL/${PLATFORM_SUBDIR}/include CACHE PATH "path to openSSL sources" FORCE)
	set(OPENSSL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/imports/OpenSSL/${PLATFORM_SUBDIR}/lib CACHE PATH "path to openSSL libraries" FORCE)
	set(OPENSSL_LIBRARIES
		${OPENSSL_LIBRARIES_DIR}/libcrypto.lib
		${OPENSSL_LIBRARIES_DIR}/libssl.lib
	)
  endif ()
elseif(APPLE)
	set(OPENSSL_BINARY_DIR ${CMAKE_SOURCE_DIR}/imports/OpenSSL-mac/bin CACHE PATH "path to openSSL dlls" FORCE)
	set(OPENSSL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/imports/OpenSSL-mac/include CACHE PATH "path to openSSL sources" FORCE)
	set(OPENSSL_LIBRARIES_DIR ${CMAKE_SOURCE_DIR}/imports/OpenSSL-mac/lib CACHE PATH "path to openSSL libraries" FORCE)
endif(WIN32)

if(NOT WIN32)
  FIND_PATH(OPENSSL_INCLUDE_DIR_TMP
    NAMES
     openssl/ssl.h
    HINTS
     ${OPENSSL_INCLUDE_DIR}
     /opt/local/include
    PATHS
     /usr/include
    PATH_SUFFIXES
     include
  )
  set(OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR_TMP})

  FIND_LIBRARY(OPENSSL_SSL_LIBRARY
    NAMES
      ssl
      ssleay32
      ssleay32MD
    HINTS
      ${OPENSSL_LIBRARIES_DIR}
      /opt/local/lib
    PATHS
      /usr/lib
      /usr/lib64
    PATH_SUFFIXES
      lib
  )

  FIND_LIBRARY(OPENSSL_CRYPTO_LIBRARY
    NAMES
      crypto
    HINTS
      ${OPENSSL_LIBRARIES_DIR}
      /opt/local/lib
    PATHS
      /usr/lib64
      /usr/lib
    PATH_SUFFIXES
      lib
  )

  set(OPENSSL_LIBRARIES
	${OPENSSL_SSL_LIBRARY}
	${OPENSSL_CRYPTO_LIBRARY}
  )
  
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(OPENSSL_SSL_LIB DEFAULT_MSG OPENSSL_SSL_LIBRARY OPENSSL_INCLUDE_DIR)
  find_package_handle_standard_args(OPENSSL_CRYPTO_LIB DEFAULT_MSG OPENSSL_CRYPTO_LIBRARY OPENSSL_INCLUDE_DIR)
endif(NOT WIN32)

macro(INSTALL_OPENSSL)
	if(WIN32)
		FILE(GLOB OPENSSL_DLLS "${OPENSSL_BINARY_DIR}/libcrypto*.dll" "${OPENSSL_BINARY_DIR}/libssl*.dll")
		install(
			FILES ${OPENSSL_DLLS}
			DESTINATION .
		)
		foreach(dllFile ${OPENSSL_DLLS})
			add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
				COMMAND
					${CMAKE_COMMAND} -E copy \"${dllFile}\" $<TARGET_FILE_DIR:${PROJECT_NAME}>
		)
		endforeach(dllFile ${OPENSSL_LIBRARIES})
	endif(WIN32)
endmacro(INSTALL_OPENSSL)
