cmake_minimum_required (VERSION 2.6)

# options
# (cmd: $ cmake -Dvar=val ...)
option(MXCFB "For i.MX target" OFF)

message("\nOptions:")
message("* MXCFB: ${MXCFB}")
message("\n")

# set adk prefix

if (MXCFB EQUAL ON)
    # by adk
    if (DEFINED ENV{ADK_PREFIX})
        set (adk_root $ENV{ADK_PREFIX}/armv7a-mv-linux)
    # by bitbake
    else (DEFINED ENV{ADK_PREFIX})
        set (adk_root ${STAGING_DIR_HOST})
    endif (DEFINED ENV{ADK_PREFIX})
endif (MXCFB EQUAL ON)

# for YCM
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# compiler flag
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (MXCFB EQUAL ON)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMXCFB")
endif (MXCFB EQUAL ON)

# include directory

include_directories(${CMAKE_SOURCE_DIR}/capture)
include_directories(${CMAKE_SOURCE_DIR}/fbdev)
include_directories(${CMAKE_SOURCE_DIR}/util)

add_executable(camera
    main.cpp
    ${CMAKE_SOURCE_DIR}/capture/ICapture.cpp
    ${CMAKE_SOURCE_DIR}/capture/ImplCapture.cpp
    ${CMAKE_SOURCE_DIR}/fbdev/IFbDev.cpp
    ${CMAKE_SOURCE_DIR}/fbdev/ImplFbDev.cpp
    ${CMAKE_SOURCE_DIR}/util/IFileParser.cpp
    ${CMAKE_SOURCE_DIR}/util/InitFileParser.cpp
    ${CMAKE_SOURCE_DIR}/util/IPixFmt.cpp
    )
