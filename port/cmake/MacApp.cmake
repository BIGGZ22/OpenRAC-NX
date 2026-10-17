# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
#
# macOS: a windowed program as an application bundle, so that macOS names it by its title.
#
# The Dock, Cmd-Tab, Force Quit and Activity Monitor name a plain executable after its file
# (openrac-rac1-pal), whatever the program says about itself once it runs, and crash reports and
# ps name any program after its executable. A program in a bundle is named by the bundle: the
# Dock and Finder by the bundle's own file name (a localized name in its Info.plist is used only
# when that matches), the rest by its Info.plist and executable, and the Dock and Finder show its
# icon before the window opens. So on macOS
#
#   openrac_macos_app(openrac-rac1-pal NAME "Ratchet & Clank (PAL)" ID dev.openrac.rac1-pal ICON <png>)
#
# builds the target as <build>/apps/openrac-rac1-pal/Ratchet & Clank (PAL).app, its program
# Contents/MacOS/Ratchet & Clank (PAL), with Contents/Info.plist (CFBundleName and CFBundleDisplayName
# NAME, CFBundleIdentifier ID) and Contents/Resources/icon.icns: ICON if it is one, else made from
# the PNG ICON (sips and iconutil, which every Mac has). A ":" in NAME, which Finder shows as "/",
# is written " -". Each bundle has a folder named after its target, where the launcher finds it
# from the target's name (launcher/core/src/lib.rs, program_path).
#
# <build>/openrac-rac1-pal stays as a link to the program, for a shell and the scripts that start
# it there: started through the link the program works the same, but macOS names it after the link.

function(openrac_macos_app target)
  cmake_parse_arguments(PARSE_ARGV 1 app "" "NAME;ID;ICON" "")
  string(REPLACE ":" " -" title "${app_NAME}")
  set(bundle "${CMAKE_BINARY_DIR}/apps/${target}/${title}.app")
  set_target_properties(${target} PROPERTIES
    OUTPUT_NAME "${title}"
    RUNTIME_OUTPUT_DIRECTORY "${bundle}/Contents/MacOS")

  string(REPLACE "&" "&amp;" name "${title}")
  string(REPLACE "<" "&lt;" name "${name}")
  file(CONFIGURE OUTPUT "${bundle}/Contents/Info.plist" CONTENT [[<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleExecutable</key>
	<string>${name}</string>
	<key>CFBundleIconFile</key>
	<string>icon</string>
	<key>CFBundleIdentifier</key>
	<string>${app_ID}</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>${name}</string>
	<key>CFBundleDisplayName</key>
	<string>${name}</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>NSHighResolutionCapable</key>
	<true/>
</dict>
</plist>
]])

  # The icon: an .icns as it is, a PNG in every size of an iconset up to 512.
  set(icns "${bundle}/Contents/Resources/icon.icns")
  if(app_ICON MATCHES "\\.icns$")
    add_custom_command(OUTPUT "${icns}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${bundle}/Contents/Resources"
      COMMAND "${CMAKE_COMMAND}" -E copy "${app_ICON}" "${icns}"
      DEPENDS "${app_ICON}"
      VERBATIM)
  else()
    set(iconset "${CMAKE_CURRENT_BINARY_DIR}/generated/${target}.iconset")
    set(resize "")
    foreach(size 16 32 128 256 512)
      math(EXPR double "${size} * 2")
      list(APPEND resize COMMAND sips -z ${size} ${size} "${app_ICON}" --out "${iconset}/icon_${size}x${size}.png")
      if(double LESS_EQUAL 512)
        list(APPEND resize
          COMMAND sips -z ${double} ${double} "${app_ICON}" --out "${iconset}/icon_${size}x${size}@2x.png")
      endif()
    endforeach()
    add_custom_command(
      OUTPUT "${icns}"
      COMMAND "${CMAKE_COMMAND}" -E rm -rf "${iconset}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${iconset}" "${bundle}/Contents/Resources"
      ${resize}
      COMMAND iconutil -c icns "${iconset}" -o "${icns}"
      DEPENDS "${app_ICON}"
      COMMENT "macOS icon for ${target}"
      VERBATIM)
  endif()

  add_custom_target(${target}_icon DEPENDS "${icns}")
  add_dependencies(${target} ${target}_icon)

  add_custom_command(TARGET ${target} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E rm -f "${CMAKE_BINARY_DIR}/${target}"
    COMMAND "${CMAKE_COMMAND}" -E create_symlink "apps/${target}/${title}.app/Contents/MacOS/${title}"
            "${CMAKE_BINARY_DIR}/${target}"
    VERBATIM)
endfunction()
