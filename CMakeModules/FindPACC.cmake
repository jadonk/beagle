# PACC finding package
message(STATUS "++ Try to find PACC libraries and headers")
if(NOT DEFINED PACC_DIR)
	message(STATUS "++ PACC_DIR not set; attempt to find PACC in default locations...")
	set(PACC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/PACC" CACHE PATH "PACC location")		# CMake will automaticaly search in system-specific directory, like /usr/lib on Linux
endif(NOT DEFINED PACC_DIR)

if(UNIX)
	set(PACC_DIR_HOME "~/PACC") 
elseif(WIN32)
	set(PACC_DIR_HOME "$ENV{UserProfile}/Mes Documents/PACC")
else(UNIX)
	set(PACC_DIR_HOME "${CMAKE_CURRENT_SOURCE_DIR}")
endif(UNIX)
message(STATUS "++ PACC_DIR set to default ${PACC_DIR} or ${PACC_DIR_HOME}")

if(NOT DEFINED PACC_LIB_DIR OR EXISTS "${PACC_DIR}")
	set(PACC_LIB_DIR "${PACC_DIR}/lib")
	set(PACC_LIB_DIR_HOME "${PACC_DIR_HOME}/lib")
	set(CMAKE_LIBRARY_PATH "${PACC_LIB_DIR_HOME}")
	mark_as_advanced(CMAKE_LIBRARY_PATH)
	message(STATUS "++ PACC_LIB_DIR set to default ${PACC_LIB_DIR} or ${PACC_LIB_DIR_HOME}")
endif(NOT DEFINED PACC_LIB_DIR OR EXISTS "${PACC_DIR}")

if(NOT DEFINED PACC_INCLUDE_DIR OR EXISTS "${PACC_DIR}")
	set(PACC_INCLUDE_DIR "${PACC_DIR}/include")
	set(PACC_INCLUDE_DIR_HOME "${PACC_DIR_HOME}/include")
	set(CMAKE_INCLUDE_PATH "${PACC_INCLUDE_DIR_HOME}")
	mark_as_advanced(CMAKE_INCLUDE_PATH)
	message(STATUS "++ PACC_INCLUDE_DIR set to default ${PACC_INCLUDE_DIR} or ${PACC_INCLUDE_DIR_HOME}")
endif(NOT DEFINED PACC_INCLUDE_DIR OR EXISTS "${PACC_DIR}")

if(EXISTS "${PACC_LIB_DIR}")
	set(PACC_LIB_FOUND_PATH PACC_LIB_FOUND_PATH-NOTFOUND)
endif(EXISTS "${PACC_LIB_DIR}")
if(EXISTS "${PACC_INCLUDE_DIR}")
	set(PACC_INCLUDE_FOUND_PATH PACC_INCLUDE_FOUND_PATH-NOTFOUND)
endif(EXISTS "${PACC_INCLUDE_DIR}")

find_library(PACC_LIB_FOUND_PATH pacc PATHS "${PACC_LIB_DIR}" NO_DEFAULT_PATH) # Try to find path to PACC library
if(PACC_LIB_FOUND_PATH MATCHES PACC_LIB_FOUND_PATH-NOTFOUND)
	find_library(PACC_LIB_FOUND_PATH pacc PATH_SUFFIXES "PACC" "PACC/lib")
endif(PACC_LIB_FOUND_PATH MATCHES PACC_LIB_FOUND_PATH-NOTFOUND)

find_path(PACC_INCLUDE_FOUND_PATH NAMES Math.hpp Socket.hpp SVG.hpp Threading.hpp Util.hpp XML.hpp PATHS ${PACC_INCLUDE_DIR} PATH_SUFFIXES "PACC" NO_DEFAULT_PATH)
if(PACC_INCLUDE_FOUND_PATH MATCHES PACC_INCLUDE_FOUND_PATH-NOTFOUND)
	find_path(PACC_INCLUDE_FOUND_PATH NAMES Math.hpp Socket.hpp SVG.hpp Threading.hpp Util.hpp XML.hpp PATH_SUFFIXES "PACC" "PACC/include/PACC")
endif(PACC_INCLUDE_FOUND_PATH MATCHES PACC_INCLUDE_FOUND_PATH-NOTFOUND)


if(PACC_LIB_FOUND_PATH MATCHES PACC_LIB_FOUND_PATH-NOTFOUND)
	message(FATAL_ERROR "Cannot find path to PACC library... please set PACC_LIB_DIR according to your PACC installation")
else(PACC_LIB_FOUND_PATH MATCHES PACC_LIB_FOUND_PATH-NOTFOUND)
	get_filename_component(PACC_LIB_FOUND_DIR ${PACC_LIB_FOUND_PATH} PATH)		# Extract the folder path
	message(STATUS "++ PACC library found : ${PACC_LIB_FOUND_DIR}")
	link_directories(${PACC_LIB_FOUND_DIR})
endif(PACC_LIB_FOUND_PATH MATCHES PACC_LIB_FOUND_PATH-NOTFOUND)

if(PACC_INCLUDE_FOUND_PATH MATCHES PACC_INCLUDE_FOUND_PATH-NOTFOUND)
	message(FATAL_ERROR "Cannot find path to PACC headers... please set PACC_INCLUDE_DIR according to your PACC installation")
else(PACC_INCLUDE_FOUND_PATH MATCHES PACC_INCLUDE_FOUND_PATH-NOTFOUND)
	string(REGEX REPLACE "PACC$" "" PACC_INCLUDE_FOUND_DIR "${PACC_INCLUDE_FOUND_PATH}")
	message(STATUS "++ PACC headers found : ${PACC_INCLUDE_FOUND_PATH}")
	include_directories(${PACC_INCLUDE_FOUND_DIR})
endif(PACC_INCLUDE_FOUND_PATH MATCHES PACC_INCLUDE_FOUND_PATH-NOTFOUND)

