# Find Torrent-Rasterbar library
#
# This module defines
#  TORRENT_FOUND - whether the qsjon library was found
#  TORRENT_LIBRARIES - the libtorrent library
#  TORRENT_INCLUDE_DIR - the include path of the libtorrent library
#

# enabled build under all OSes
	set(TORRENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/3rdparty/torrent-rasterbar/include)
	add_subdirectory(${CMAKE_SOURCE_DIR}/src/3rdparty/torrent-rasterbar)
	set(TORRENT_LIBRARIES torrent-rasterbar)
#	add_definitions(-DBOOST_ALL_NO_LIB)
#	add_definitions(-D__USE_W32_SOCKETS -DWIN32_LEAN_AND_MEAN)
	add_definitions(-DTORRENT_DISABLE_GEO_IP)
	add_definitions(-DUNICODE -D_UNICODE)
	#add_definitions(-DTORRENT_NO_DEPRECATE)
	add_definitions(-DTORRENT_USE_OPENSSL)

	if(DEVELOPER_TORRENTS_LOGS)
		add_definitions(-DTORRENT_LOGGING)
		add_definitions(-DTORRENT_VERBOSE_LOGGING)
	endif(DEVELOPER_TORRENTS_LOGS)


#add_definitions(-DBOOST_ASIO_HASH_MAP_BUCKETS=1021 -DBOOST_ASIO_SEPARATE_COMPILATION)
# using paths for compilation
include_directories(${Boost_INCLUDE_DIR})
include_directories(${OPENSSL_INCLUDE_DIR})


  # Already in cache
  set (TORRENT_FOUND TRUE)
