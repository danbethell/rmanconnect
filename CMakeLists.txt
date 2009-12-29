cmake_minimum_required( VERSION 2.6 )
project( rmanconnect )

set( NDKDIR "/usr/local/Nuke5.2v3" )
set( DELIGHT "/usr/local/3delight-9.0.0/Linux-x86_64" )
set( ILMBASE "/usr/include/OpenEXR" )

find_package( Boost 1.40.0 COMPONENTS system REQUIRED )

include_directories( 
  ${CMAKE_SOURCE_DIR}/src
  ${NDKDIR}/include
  ${DELIGHT}/include
  ${ILMBASE}
  ${Boost_INCLUDE_DIRS}
  )

link_directories( 
  ${NDKDIR} 
  ${DELIGHT}/lib
  )

add_library( nuke_plugin 
  SHARED 
  ${CMAKE_SOURCE_DIR}/src/nk_rmanConnect.cpp 
  ${CMAKE_SOURCE_DIR}/src/Server.cpp
  ${CMAKE_SOURCE_DIR}/src/Data.cpp
  )

set_target_properties( nuke_plugin
  PROPERTIES
  PREFIX ""
  OUTPUT_NAME "nk_rmanConnect"
  COMPILE_FLAGS "-DUSE_GLEW"
  )

target_link_libraries( nuke_plugin 
  DDImage
  Iex
  ${Boost_LIBRARIES}
  )

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
  SUFFIX ".dpy"
  COMPILE_FLAGS ""
)

target_link_libraries( rman_plugin
  3delight
  Iex
  ${Boost_LIBRARIES}
  )