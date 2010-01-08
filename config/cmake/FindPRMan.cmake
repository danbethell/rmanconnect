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
#   PRMan_FOUND    
#   PRMan_INCLUDE_DIR
#   PRMan_COMPILE_FLAGS
#   PRMan_LIBRARIES
#   PRMan_LIBRARY_DIR
#
# Usage: 
#   FIND_PACKAGE( PRMan )
#   FIND_PACKAGE( PRMan REQUIRED )
#
# Note:
# You can tell the module where PRMan is installed by setting
# the PRMan_INSTALL_PATH (or setting the RMANTREE environment
# variable) before calling FIND_PACKAGE.
# 
# E.g. 
#   SET( PRMan_INSTALL_PATH "/opt/pixar/RenderManProServer-15.0" )
#   FIND_PACKAGE( PRMan REQUIRED )
#
#==========

# our includes
FIND_PATH( PRMan_INCLUDE_DIR ri.h
  $ENV{RMANTREE}/include
  ${PRMan_INSTALL_PATH}/include
  )

# our library itself
FIND_LIBRARY( PRMan_LIBRARIES prman
  $ENV{RMANTREE}/lib
  ${PRMan_INSTALL_PATH}/lib
  )

# our compilation flags
SET( PRMan_COMPILE_FLAGS "-DPRMAN -fPIC" )

# did we find everything?
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( "PRMan" DEFAULT_MSG
  PRMan_INCLUDE_DIR
  PRMan_LIBRARIES
  )
