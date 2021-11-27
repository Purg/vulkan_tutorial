#
# Encapsulation for setting appropriate compiler flags depending on the compiler being used.
#
# Thanks KWIVER for the baseline (which either Ben or I wrote in the first place...).
#

include( CheckCXXCompilerFlag )

define_property( GLOBAL PROPERTY global_prop_project_cxx_flags
  BRIEF_DOCS "Warning flags for project build"
  FULL_DOCS "List of warning flags project will build with."
  )

#
# Helper function for checking the validity of a compiler flag, and then adding
# it to a global list if the check is successful.
#
# Input flags are checked for validity with the CMake-included
# `check_cxx_compiler_flag` functionality.
#
# If a list of flags are passed, the first valid flag is added.
# This is useful when you are looking for the highest level of compiler support
# (e.g. `(-std=c++11 -std=c++0x)` will implicitly set the flag for the highest
# level of support).
#
# If this check fails for all input flags, the flags are not added and a FATAL_ERROR message is
# output.
#
function( project_check_compiler_flag )
  foreach( flag ${ARGN} )
    string( REPLACE "+" "plus" safeflag "${flag}" )
    string( REPLACE "/" "slash" safeflag "${safeflag}" )
    check_cxx_compiler_flag( "${flag}" "has_compiler_flag-${safeflag}" )
    if( has_compiler_flag-${safeflag} )
      set_property( GLOBAL APPEND PROPERTY global_prop_project_cxx_flags "${flag}" )
      return()
    endif()
  endforeach()
  if( "${ARGC}" GREATER 0 )
    # If we're here then >0 flags were passed and none of them passed the check.
    message( FATAL_ERROR "Failed to validate compiler flag(s): ${flags}" )
  endif()
endfunction()


# Descend into the appropriate flag checks based on the detected compiler being
# used.
#
# TODO: Encapsulate sections into sub-modules if they grow much beyond a few
#       lines or add additional logic.
if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
  message( WARNING "No explicit flag support for MSVC yet." )
elseif( CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
  message( WARNING "No explicit flag support for Clang yet." )
elseif( CMAKE_CXX_COMPILER_ID MATCHES "GNU" )
  project_check_compiler_flag( -Wall )
endif()


get_property( project_cxx_flags GLOBAL PROPERTY "global_prop_project_cxx_flags" )
string( REPLACE ";" " " project_cxx_flags "${project_cxx_flags}" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${project_cxx_flags}" )
message( STATUS "Using CXX flags after checks: ${CMAKE_CXX_FLAGS}" )
