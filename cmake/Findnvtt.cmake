include(FindPackageHandleStandardArgs)

find_path(NVTT_INCLUDE_DIR
  NAMES nvtt/nvtt.h
)

find_library(NVTT_LIBRARY
    NAMES nvtt
    PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static static
)

find_package_handle_standard_args(nvtt
  REQUIRED_VARS NVTT_INCLUDE_DIR NVTT_LIBRARY
)

if(nvtt_FOUND AND NOT TARGET Nvidia::nvtt)
    add_library(Nvidia::nvtt UNKNOWN IMPORTED)
    set_target_properties(Nvidia::nvtt PROPERTIES
      IMPORTED_LOCATION "${NVTT_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${NVTT_INCLUDE_DIR}"
    )

    # vcpkg's nvtt port ships the library split into several static archives.
    # The nvtt.lib references symbols from the others, hence all of them must be linked together.
    set(_nvtt_component_names nvcore nvimage nvmath nvsquish nvthread bc6h bc7)

    foreach(_comp IN LISTS _nvtt_component_names)
      find_library(${_comp}_LIBRARY
        NAMES ${_comp}
        PATH_SUFFIXES lib64 lib lib/shared lib/static lib64/static static
      )
    endforeach()

    set(_nvtt_deps)
    foreach(_comp IN LISTS _nvtt_component_names)
      if(${_comp}_LIBRARY)
        list(APPEND _nvtt_deps "${${_comp}_LIBRARY}")
      endif()
    endforeach()
    if(_nvtt_deps)
      set_target_properties(Nvidia::nvtt PROPERTIES
        INTERFACE_LINK_LIBRARIES "${_nvtt_deps}"
      )
    endif()
endif()