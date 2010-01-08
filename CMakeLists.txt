#==========
#
# Copyright (c) 2010, Dan Bethell, Johannes Saam.
# All rights reserved.
#
# For license information regarding redistribution and
# use, please refer to the COPYING file.
#
#==========

cmake_minimum_required( VERSION 2.6 )
project( rmanconnect )

#=====
# General
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/config/cmake )
find_package( Boost 1.40.0 COMPONENTS system REQUIRED )
find_package( Nuke REQUIRED )
find_package( Doxygen )

# try to find a rman lib (set by the RMAN envvar)
set( RMAN "3Delight" )
if ( $ENV{RMAN} MATCHES "" )
  set( RMAN $ENV{RMAN} )
endif ( $ENV{RMAN} MATCHES "" )
find_package( ${RMAN} REQUIRED )

include_directories(
  ${CMAKE_SOURCE_DIR}/src
  ${Boost_INCLUDE_DIRS}
  ${Nuke_INCLUDE_DIR}
  ${${RMAN}_INCLUDE_DIR}
  )

link_directories( 
  ${Nuke_LIBRARY_DIR} # we add this so we can find Nuke's Iex lib
  )

#=====
# Build the Nuke plugin
add_library( nuke_plugin 
  SHARED
  ${CMAKE_SOURCE_DIR}/src/nk_rmanConnect.cpp 
  ${CMAKE_SOURCE_DIR}/src/Server.cpp
  ${CMAKE_SOURCE_DIR}/src/Client.cpp
  ${CMAKE_SOURCE_DIR}/src/Data.cpp
  )

set_target_properties( nuke_plugin
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "nk_rmanConnect"
  COMPILE_FLAGS "-DUSE_GLEW"
  )

target_link_libraries( nuke_plugin 
  ${Boost_LIBRARIES}
  ${Nuke_LIBRARIES}
  Iex
  )

#=====
# Build the Rman plugin
add_library( rman_plugin
  SHARED
  ${CMAKE_SOURCE_DIR}/src/d_rmanConnect.cpp
  ${CMAKE_SOURCE_DIR}/src/Client.cpp
  ${CMAKE_SOURCE_DIR}/src/Data.cpp
  )

set_target_properties( rman_plugin
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "d_rmanConnect"
  COMPILE_FLAGS "${${RMAN}_COMPILE_FLAGS}"
)

target_link_libraries( rman_plugin
  ${Boost_LIBRARIES}
  ${${RMAN}_LIBRARIES}
  Iex
  )

#=====
# Build docs (after the nuke plugin is built)
IF( DOXYGEN_FOUND )
    ADD_CUSTOM_COMMAND( TARGET nuke_plugin
        POST_BUILD
        COMMAND ${DOXYGEN_EXECUTABLE} 
        ${CMAKE_SOURCE_DIR}/"config/docs/doxygen.cfg" 
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
ENDIF( DOXYGEN_FOUND )
