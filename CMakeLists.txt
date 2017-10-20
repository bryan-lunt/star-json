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
set(BUILD_SHARED_LIBRARIES OFF)

find_path(JANSSON_INCLUDE_FOUND jansson.h HINTS ${JANSSON_INCLUDE} "/usr/include" "/data/data127/pvicente/install/jansson-2.9/include")
if(NOT JANSSON_INCLUDE_FOUND)
  message(FATAL_ERROR "jansson.h header file not found")
else()
  message("-- Found jansson.h header file at: " ${JANSSON_INCLUDE_FOUND})
endif()

find_library(JANSSON_LIBRARY NAMES jansson HINTS "/data/data127/pvicente/install/jansson-2.9/lib/")
if(NOT JANSSON_LIBRARY)
  message(FATAL_ERROR "jansson library not found")
else()
  message("-- Found jansson library at: " ${JANSSON_LIBRARY})
endif()

find_path(HDF5_INCLUDE_FOUND hdf5.h HINTS ${HDF5_INCLUDE} "/usr/include/hdf5/serial" "/data/starfs1/libs/hdf5-1.8.7-linux-x86_64-static/include")
if(NOT HDF5_INCLUDE_FOUND)
  message(FATAL_ERROR "hdf5.h header file not found")
else()
  message("-- Found hdf5.h header file at: " ${HDF5_INCLUDE_FOUND})
endif()

set(LIB_HDF5_HINTS ${LIB_HDF5_HINTS} "/usr/lib/i386-linux-gnu/hdf5/serial")
set(LIB_HDF5_HINTS ${LIB_HDF5_HINTS} "/data/starfs1/libs/hdf5-1.8.7-linux-x86_64-static/lib")
find_library(HDF5_LIBRARY NAMES hdf5 HINTS ${LIB_HDF5_HINTS})
if(NOT HDF5_LIBRARY)
  message(FATAL_ERROR "hdf5 library not found")
else()
  message("-- Found hdf5 library at: " ${HDF5_LIBRARY})
endif()

find_library(ZLIB_LIBRARY NAMES z HINTS "/data/starfs1/libs/hdf5-1.8.7-linux-x86_64-static/lib")
if(NOT ZLIB_LIBRARY)
  message(FATAL_ERROR "zlib library not found")
else()
  message("-- Found zlib library at: " ${ZLIB_LIBRARY})
endif()

find_library(SZIP_LIBRARY NAMES sz HINTS "/data/starfs1/libs/hdf-4.2.6-linux-x86_64/lib/" "/root/usr2/MRT/lib")
if(NOT SZIP_LIBRARY)
  message(FATAL_ERROR "szip library not found")
else()
  message("-- Found szip library at: " ${SZIP_LIBRARY})
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

