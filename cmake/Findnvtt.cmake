include(FindPackageHandleStandardArgs)

find_path(NVTT_INCLUDE_DIR
  NAMES nvtt/nvtt.h
)

# vcpkg's nvtt port ships the library split into several static archives
# under <prefix>/lib/static/. nvtt.lib references symbols from the others,
# so all of them must be linked together.
set(_nvtt_components nvtt nvcore nvimage nvmath nvsquish nvthread bc6h bc7)

foreach(_comp IN LISTS _nvtt_components)
  find_library(NVTT_${_comp}_LIBRARY
    NAMES ${_comp}
    PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static static
  )
endforeach()

# Keep NVTT_LIBRARY exposed for callers that referenced the old variable name.
set(NVTT_LIBRARY "${NVTT_nvtt_LIBRARY}")

find_package_handle_standard_args(nvtt
  REQUIRED_VARS NVTT_INCLUDE_DIR NVTT_nvtt_LIBRARY
)

if(nvtt_FOUND AND NOT TARGET Nvidia::nvtt)
  # UNKNOWN IMPORTED so IMPORTED_LOCATION is honored (an INTERFACE IMPORTED
  # target silently ignores IMPORTED_LOCATION and won't actually link).
  add_library(Nvidia::nvtt UNKNOWN IMPORTED)
  set_target_properties(Nvidia::nvtt PROPERTIES
    IMPORTED_LOCATION "${NVTT_nvtt_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${NVTT_INCLUDE_DIR}"
  )

  set(_nvtt_deps)
  foreach(_comp IN LISTS _nvtt_components)
    if(NOT _comp STREQUAL "nvtt" AND NVTT_${_comp}_LIBRARY)
      list(APPEND _nvtt_deps "${NVTT_${_comp}_LIBRARY}")
    endif()
  endforeach()
  if(_nvtt_deps)
    set_target_properties(Nvidia::nvtt PROPERTIES
      INTERFACE_LINK_LIBRARIES "${_nvtt_deps}"
    )
  endif()
endif()