# Copyright © 2021 CCP ehf.

cmake_minimum_required(VERSION 3.21)
project(ImageTools VERSION 1.0.0)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CcpBuildConfigurations)
include(CcpTargetConfigurations)
include(CcpPackageConfigHelpers)
include(CcpMonolithVendorConfig)

find_package(carbon-blueexposure REQUIRED CONFIG COMPONENTS BlueExposure)
find_package(carbon-math REQUIRED CONFIG COMPONENTS CcpMath)
find_package(carbon-imageio REQUIRED CONFIG COMPONENTS CcpImageIO)
find_package(nvtt REQUIRED)


find_package(libjpeg-turbo CONFIG REQUIRED)
find_package(PNG CONFIG REQUIRED)

ccp_add_library(ImageTools SHARED)

target_link_libraries(ImageTools PRIVATE
    BlueExposure
    CcpMath
    CcpImageIO
    Nvidia::nvtt
)

target_sources(ImageTools PRIVATE
    AllowThreads.h
    CompressionOptions.cpp
    CompressionOptions.h
    CompressionOptions_Blue.cpp
    EnumRegistry_Blue.cpp
    FileStream.cpp
    FileStream.h
    GenerateRoughness_Blue.cpp
    ImageIOResultBlue.cpp
    ImageIOResultBlue.h
    ImageTools.cpp
    ImageToolsBitmap.cpp
    ImageToolsBitmap.h
    ImageToolsBitmap_Blue.cpp
    MemoryOutputHandler.cpp
    MemoryOutputHandler.h
    MemoryStream.cpp
    MemoryStream.h
    NanoVDBToolSupport_Blue.cpp
    NanoVDBToolSupport.h
    StdAfx.cpp
    StdAfx.h
)

#target_include_directories(ImageTools PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ImageTools PRIVATE BLUEBUILD_STATIC)

if(APPLE)
    set_target_properties(
        ImageTools
        PROPERTIES 
            PREFIX ""
            SUFFIX ".so"
    )
endif()
set_target_properties(ImageTools PROPERTIES OUTPUT_NAME "_imagetools")


# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)

    option(INSTALL_TO_MONOLITH "Install this package to a perforce branch vendor folder - provided for backwards compatability" OFF)

    if(INSTALL_TO_MONOLITH)
        # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
        install(
            TARGETS ImageTools
            EXPORT ImageToolsTargets
            CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
            LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
            ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
            RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        )

        message(STATUS "lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/")

        configure_ccp_vendor_config_file(
            TARGET ImageTools
            DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/ImageToolsConfig.cmake"
            DEPENDENCIES ""
        )
        install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ImageToolsConfig.cmake DESTINATION .)

        install(IMPORTED_RUNTIME_ARTIFACTS libjpeg-turbo::jpeg DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/)
        install(IMPORTED_RUNTIME_ARTIFACTS PNG::png_shared DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/)

    else()
        install(
            TARGETS ImageTools
            EXPORT ImageToolsTargets
            CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
            LIBRARY DESTINATION lib/
            ARCHIVE DESTINATION lib/
            RUNTIME DESTINATION bin/
        )

        install(
            EXPORT ImageToolsTargets
            DESTINATION share/carbon-imagetools
            FILE carbon-imagetoolsConfig.cmake
        )

    endif()

    # Install python package
     install(DIRECTORY python/ DESTINATION python)
endif()

