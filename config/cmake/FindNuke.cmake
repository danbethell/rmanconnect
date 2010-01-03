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
#   Nuke_FOUND    
#   Nuke_INCLUDE_DIR
#   Nuke_LIBRARIES
#   Nuke_LIBRARY_DIR
#
# Usage: 
#   FIND_PACKAGE( Nuke )
#   FIND_PACKAGE( Nuke REQUIRED )
#
# Note:
# You can tell the module where Nuke is installed by setting
# the Nuke_INSTALL_PATH (or setting the NDK_PATH environment
# variable before calling FIND_PACKAGE.
# 
# E.g. 
#   SET( Nuke_INSTALL_PATH "/usr/local/Nuke5.2v3" )
#   FIND_PACKAGE( Nuke REQUIRED )
#
#==========

# our includes
FIND_PATH( Nuke_INCLUDE_DIR DDImage/Op.h
  $ENV{NDK_PATH}/include
  ${Nuke_INSTALL_PATH}/include
  )

# our library
FIND_LIBRARY( Nuke_LIBRARIES DDImage
  $ENV{NDK_PATH}
  ${Nuke_INSTALL_PATH}
  )

# our library path
GET_FILENAME_COMPONENT( Nuke_LIBRARY_DIR ${Nuke_LIBRARIES} PATH )

# did we find everything?
SET( Nuke_FOUND "NO" )
IF( Nuke_INCLUDE_DIR )
  IF( Nuke_LIBRARIES )
    SET( Nuke_FOUND "YES" )
    MESSAGE(STATUS "Found Nuke" )
  ENDIF( Nuke_LIBRARIES )
ENDIF( Nuke_INCLUDE_DIR )

IF( Nuke_FIND_REQUIRED )
  IF( NOT Nuke_FOUND )
    MESSAGE(FATAL_ERROR "Could not find REQUIRED Nuke!" )
  ENDIF( NOT Nuke_FOUND )
ENDIF( Nuke_FIND_REQUIRED )
