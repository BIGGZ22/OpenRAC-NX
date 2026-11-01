# SPDX-License-Identifier: GPL-3.0-or-later
include_guard(GLOBAL)

# A loaderless, self-contained NVK archive with internal Mesa symbols localized.
# Do not mix an unlocalized Mesa archive with the portlibs' OpenGL Mesa libraries.
function(openrac_switch_vulkan_dependencies target)
  # Volk's dynamic-loader path includes dlfcn.h, which libnx does not provide.
  # Generate a static-only copy; preserve upstream source and license untouched.
  file(READ "${volk_SOURCE_DIR}/volk.c" volk_source)
  string(FIND "${volk_source}" "VkResult volkInitialize(void)" init_begin)
  string(FIND "${volk_source}" "void volkInitializeCustom(" init_end)
  if(init_begin LESS 0 OR init_end LESS init_begin)
    message(FATAL_ERROR "Pinned volk source no longer matches the Switch static-loader adaptation")
  endif()
  string(SUBSTRING "${volk_source}" 0 ${init_begin} prefix)
  string(SUBSTRING "${volk_source}" ${init_end} -1 suffix)
  set(volk_source "${prefix}VkResult volkInitialize(void) { return VK_ERROR_INITIALIZATION_FAILED; }\n\n${suffix}")
  string(REPLACE "#\tinclude <dlfcn.h>" "/* Switch uses volkInitializeCustom, without libdl. */" volk_source "${volk_source}")
  string(REPLACE "dlclose(loadedModule);" "(void)loadedModule;" volk_source "${volk_source}")
  set(static_volk "${CMAKE_CURRENT_BINARY_DIR}/generated/volk_switch.c")
  file(CONFIGURE OUTPUT "${static_volk}" CONTENT "${volk_source}" @ONLY)
  set_property(TARGET ${target} PROPERTY SOURCES "${static_volk}")
  set(OPENRAC_SWITCH_VULKAN_LIBRARY "" CACHE FILEPATH "Self-contained Switch NVK libvulkan.a")
  if(NOT EXISTS "${OPENRAC_SWITCH_VULKAN_LIBRARY}")
    message(FATAL_ERROR "Experimental Switch Vulkan needs -DOPENRAC_SWITCH_VULKAN_LIBRARY=/path/to/libvulkan.a. See docs/port/SWITCH_VULKAN.md.")
  endif()
  add_library(openrac_switch_nvk STATIC IMPORTED GLOBAL)
  set_target_properties(openrac_switch_nvk PROPERTIES IMPORTED_LOCATION "${OPENRAC_SWITCH_VULKAN_LIBRARY}")
  # NVK exports a function with this name; volk normally defines a global pointer
  # with the same name. Rename only volk's pointer, including all its consumers.
  target_compile_definitions(${target} PUBLIC VK_USE_PLATFORM_VI_NN
    vkGetInstanceProcAddr=openrac_vkGetInstanceProcAddr)
  target_link_libraries(${target} PUBLIC openrac_switch_nvk)
endfunction()
