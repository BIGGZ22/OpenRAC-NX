# SPDX-License-Identifier: MIT
# The SDK's version is the mod interface's (openrac_mod.h, OPENRAC_MOD_API_MAJOR.MINOR): a request
# for 1.x is met by any 1.y with y >= x, never by another major.
#
# CMake reads this file for each find_package(OpenRACMod <version>) and sets PACKAGE_VERSION_* from
# what it says. Keep PACKAGE_VERSION equal to OPENRAC_MOD_API_MAJOR.MINOR in
# include/openrac/openrac_mod.h.
set(PACKAGE_VERSION "1.4")
# Another major is a different interface: the headers' ABI changed.
if(PACKAGE_FIND_VERSION_MAJOR AND NOT PACKAGE_FIND_VERSION_MAJOR EQUAL 1)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
# A newer minor than this SDK's asks for something its headers do not have.
elseif(PACKAGE_FIND_VERSION VERSION_GREATER PACKAGE_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION VERSION_EQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
