#  Copyright (c) Continental AG and subsidiaries 2017
#  All rights reserved
#
#  The reproduction, transmission or use of this document or its contents is
#  not permitted without express written authority.
#  Offenders will be liable for damages. All rights, including rights created
#  by patent grant or registration of a utility model or design, are reserved.
##########################

cmake_minimum_required (VERSION 2.6)
project (fb_switch)
set (exe fb_switch)

# Check if trigered by bitbake
# (if triggered manually, "ADK_PREFIX" env.var. is always defined)
if (DEFINED ENV{ADK_PREFIX})
    set (adk_root $ENV{ADK_PREFIX}/armv7a-mv-linux)
else (DEFINED ENV{ADK_PREFIX})
    set (adk_root ${STAGING_DIR_HOST})
endif (DEFINED ENV{ADK_PREFIX})

# export compile commands
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# where to find headers
include_directories(${PROJECT_SOURCE_DIR}/../video_service/FbControl/fbbl/
                    ${adk_root}/usr/include/
                    ${PROJECT_SOURCE_DIR}/../video_service/FbControl/fbdev/
                    )
                    
# where to find libs
link_directories(${adk_root}/usr/lib/
    ${PROJECT_SOURCE_DIR}/../video_service/FbControl/build
)

# source files
add_executable(${exe}
${PROJECT_SOURCE_DIR}/demo.cpp
)

# link
target_link_libraries(${exe} FbDevBL)

# install (just for removing rpath)
install (TARGETS ${exe} RUNTIME DESTINATION ${PROJECT_SOURCE_DIR} )
