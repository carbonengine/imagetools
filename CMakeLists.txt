project(ImageTools VERSION 1.0.0)

find_package(nvtt REQUIRED)

add_library(ImageTools SHARED)
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
    StdAfx.cpp
    StdAfx.h
)

target_include_directories(ImageTools PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)
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
