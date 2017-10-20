#options
#JANSSON_INCLUDE
#HDF5_INCLUDE
#JANSSON_LIBRARY
#HDF5_LIBRARY
#ZLIB_LIBRARY
#SZIP_LIBRARY

cmake_minimum_required(VERSION 2.8)
project (star_json)
include(CheckCXXCompilerFlag)
include(CheckFunctionExists)
include(CheckIncludeFile)
include(CheckLibraryExists)

#/////////////////////////////////////////////////////////////////////////////////////
#general C/C++ build flags
#/////////////////////////////////////////////////////////////////////////////////////

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_STATIC_LIBS ON)
set(BUILD_SHARED_LIBS OFF)

#//////////////////////////
# colorized output
# red, fatal error, cannot build (missing netCDF, HDF5, CURL libraries)
# magenta, "negative" warning (missing optional libraries, GSL, ANTLR, UDUNITS)
# green, "positive" warning (opposite of magenta, optional libraries found)
# blue, general information to take notice (SZIP/ZLIB were detected as needed, special NCO functions are defined)
#//////////////////////////

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(color_reset   "${Esc}[m")
  set(color_magenta "${Esc}[35m")
  set(color_red     "${Esc}[31m")
  set(color_green   "${Esc}[32m")
  set(color_blue    "${Esc}[34m")
endif()

#//////////////////////////
#detect dependencies using
#find_path()
#find_library()
#Notes:
#1) list a static name first to avoid link errors in systems with multiple versions of the same libraries in PATH, used for libz.a
#2) use option NO_SYSTEM_ENVIRONMENT_PATH
#//////////////////////////

set(find_opt NO_SYSTEM_ENVIRONMENT_PATH)

find_path(JANSSON_INCLUDE_FOUND jansson.h HINTS ${JANSSON_INCLUDE})
if(NOT JANSSON_INCLUDE_FOUND)
  message(FATAL_ERROR "jansson.h header file not found")
else()
  message("-- Found jansson.h header file at: " ${JANSSON_INCLUDE_FOUND})
endif()

find_library(JANSSON_LIBRARY NAMES jansson)
if(NOT JANSSON_LIBRARY)
  message(FATAL_ERROR "jansson library not found")
else()
  message("-- Found jansson library at: " ${JANSSON_LIBRARY})
endif()

#/////////////////////////////////////////////////////////////////////////////////////
#HDF5
#/////////////////////////////////////////////////////////////////////////////////////

find_path(HDF5_INCLUDE_FOUND hdf5.h HINTS ${HDF5_INCLUDE})
if(NOT HDF5_INCLUDE_FOUND)
  message(FATAL_ERROR "hdf5.h header file not found")
else()
  message("-- Found hdf5.h header file at: " ${HDF5_INCLUDE_FOUND})
endif()

find_library(HDF5_LIBRARY NAMES hdf5 ${find_opt})
if(NOT HDF5_LIBRARY)
  message(FATAL_ERROR "${color_red}hdf5 library not found${color_reset}")
else()
  message("-- Found hdf5 library at: " ${HDF5_LIBRARY})
endif()

find_library(HDF5_HL_LIBRARY NAMES hdf5_hl ${find_opt})
if(NOT HDF5_HL_LIBRARY)
  message(FATAL_ERROR "${color_red}hdf5 high level library not found${color_reset}")
else()
  message("-- Found hdf5 high level library at: " ${HDF5_HL_LIBRARY})
endif()

#//////////////////////////
#HDF5 can be optionally linked with the SZIP library (Science Data Lossless Compression Program) and ZLIB
#Symbol to detect in ZLIB can be only H5Z_DEFLATE, a structure only defined in H5Zdeflate.c if the filter is enabled
#For SZIP the structure can be only H5Z_SZIP, defined in H5Zszip.c if the filter is enabled
#check_library_exists() tries to link a temporary program with these symbols
#On MSVC for this detection to work, the HDF5 library MUST HAVE as additional dependencies the ZLIB and SZIP libraries,
#which is not a requirement for the library to build successfully
#//////////////////////////

set(CMAKE_REQUIRED_LIBRARIES ${HDF5_LIBRARY})

message("-- Detecting if HDF5 library ${HDF5_LIBRARY} needs the ZLIB library...")
check_library_exists(${HDF5_LIBRARY} H5Z_DEFLATE "" NEED_ZLIB)
if (NEED_ZLIB)
  message("${color_blue}-- ZLIB library is needed...${color_reset}")
else()
  message("${color_blue}-- ZLIB library is not needed...${color_reset}")
endif()

message("-- Detecting if HDF5 library ${HDF5_LIBRARY} needs the SZIP library...")
check_library_exists(${HDF5_LIBRARY} H5Z_SZIP "" NEED_SZIP)
if (NEED_SZIP)
  message("${color_blue}-- SZIP library is needed...${color_reset}")
else()
  message("${color_blue}-- SZIP library is not needed...${color_reset}")
endif()

if (NEED_SZIP)
  message("-- Trying to find the SZIP library...")
  find_library(SZIP_LIBRARY NAMES sz ${find_opt})
  if(NOT SZIP_LIBRARY)
    message(FATAL_ERROR "${color_red}szip library not found${color_reset}")
  else()
    message("-- Found szip library at: " ${SZIP_LIBRARY})
  endif()
endif()

if (NEED_ZLIB)
  message("-- Trying to find the ZLIB library...")
  find_library(ZLIB_LIBRARY NAMES libz.a z ${find_opt})
  if(NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "${color_red}zlib library not found${color_reset}")
  else()
    message("-- Found zlib library at: " ${ZLIB_LIBRARY})
  endif()
endif()



if (MSVC)
  add_definitions( "/D_CRT_SECURE_NO_WARNINGS /D_WINSOCK_DEPRECATED_NO_WARNINGS" )
  if (STATIC_CRT)
    message("-- Building with static runtime library")
	set(CMAKE_CXX_FLAGS_DEBUG "/D_DEBUG /MTd /Zi /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "/MT /O1 /Ob1 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "/MT /O2 /Ob2 /D NDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/MT /Zi /O2 /Ob1 /D NDEBUG")
  endif()  
  set(WINSOCK_LIBRARY ws2_32.lib)
endif(MSVC)

if (CMAKE_VERSION VERSION_LESS "3.1")
 if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
   CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
   if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
   elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
   else()
    message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
   endif()
 endif()
else ()
  set (CMAKE_CXX_STANDARD 11)
endif()

include_directories(${JANSSON_INCLUDE_FOUND} ${HDF5_INCLUDE_FOUND})
set(SOURCES star_json.cc star_json.hh)
add_executable(star_json star_json_main.cc ${SOURCES})
target_link_libraries (star_json ${JANSSON_LIBRARY} ${HDF5_LIBRARY} ${ZLIB_LIBRARY} ${SZIP_LIBRARY} ${WINSOCK_LIBRARY})

