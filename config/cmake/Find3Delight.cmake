#==========
#
# Copyright (c) 2010, Dan Bethell.
# All rights reserved.
#
# For license information regarding redistribution and
# use, please refer to the COPYING file.
#
#==========
#
# Variables defined by this module:
#   3Delight_FOUND    
#   3Delight_INCLUDE_DIR
#   3Delight_COMPILE_FLAGS
#   3Delight_LIBRARIES
#   3Delight_LIBRARY_DIR
#
# Usage: 
#   FIND_PACKAGE( 3Delight )
#   FIND_PACKAGE( 3Delight REQUIRED )
#
# Note:
# You can tell the module where 3Delight is installed by setting
# the 3Delight_INSTALL_PATH (or setting the DELIGHT environment
# variable before calling FIND_PACKAGE.
# 
# E.g. 
#   SET( 3Delight_INSTALL_PATH "/usr/local/3delight-9.0.0/Linux-x86_64" )
#   FIND_PACKAGE( 3Delight REQUIRED )
#
#==========

# our includes
FIND_PATH( 3Delight_INCLUDE_DIR ri.h
  $ENV{DELIGHT}/include
  ${3Delight_INSTALL_PATH}/include
  )

# our compilation flags
SET( 3Delight_COMPILE_FLAGS "-DDELIGHT" )

# our library itself
FIND_LIBRARY( 3Delight_LIBRARIES 3delight
  $ENV{DELIGHT}/lib
  ${3Delight_INSTALL_PATH}/lib
  )

# our library path
GET_FILENAME_COMPONENT( 3Delight_LIBRARY_DIR ${3Delight_LIBRARIES} PATH )

# did we find everything?
SET( 3Delight_FOUND "NO" )
IF( 3Delight_INCLUDE_DIR )
  IF( 3Delight_LIBRARIES )
    SET( 3Delight_FOUND "YES" )
    MESSAGE(STATUS "Found 3Delight" )
  ENDIF( 3Delight_LIBRARIES )
ENDIF( 3Delight_INCLUDE_DIR )

IF( 3Delight_FIND_REQUIRED )
  IF( NOT 3Delight_FOUND )
    MESSAGE(FATAL_ERROR "Could not find REQUIRED 3Delight!" )
  ENDIF( NOT 3Delight_FOUND )
ENDIF( 3Delight_FIND_REQUIRED )
