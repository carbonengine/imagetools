cmake_minimum_required(VERSION 3.16)
project(ImageTools VERSION 1.0.0)

if(NOT DEFINED ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "Missing required environment variable CCP_EVE_PERFORCE_BRANCH_PATH")
elseif(NOT IS_ABSOLUTE $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be an absolute path")
elseif(NOT EXISTS $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH is pointing at a non-existing location $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
elseif(NOT IS_DIRECTORY $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be a directory")
else()
    message(STATUS "Using CCP_EVE_PERFORCE_BRANCH_PATH located at $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
endif()

file(TO_CMAKE_PATH "$ENV{CCP_EVE_PERFORCE_BRANCH_PATH}" BRANCH_ROOT_DIR)
list(INSERT CMAKE_MODULE_PATH 0 ${BRANCH_ROOT_DIR}/cmake)
include(CcpGlobalSettings)
include(CcpPackageConfigHelpers)

find_package(BlueExposure REQUIRED NO_CMAKE_PATH)
find_package(CcpMath REQUIRED NO_CMAKE_PATH)
find_package(ImageIO REQUIRED NO_CMAKE_PATH)
find_package(nvtt REQUIRED)

find_package(JPEG REQUIRED NO_CMAKE_PATH)
find_package(PNG REQUIRED NO_CMAKE_PATH)
find_package(ZLIB REQUIRED NO_CMAKE_PATH)
find_package(NanoVDB REQUIRED NO_CMAKE_PATH)

ccp_add_library(ImageTools SHARED)

target_link_libraries(ImageTools PRIVATE
    BlueExposure
    CcpMath
    ImageIO
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
    # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
    install(
        TARGETS ImageTools
        EXPORT ImageToolsTargets
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )
    # Install rule for available public headers
    install(FILES ${PUBLIC_HEADER_FILES} DESTINATION include)

    message(STATUS "lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/")

    configure_ccp_vendor_config_file(
        TARGET ImageTools
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/ImageToolsConfig.cmake"
        DEPENDENCIES ""
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/ImageToolsConfig.cmake DESTINATION .)

     # Install python package
     install(DIRECTORY python/ DESTINATION python
     FILES_MATCHING
     PATTERN "*.py"
)
endif()

