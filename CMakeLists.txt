#
#  CMakeLists.txt
#  lockfree_container
#
#  Created by Dmitrii Torkhov <dmitriitorkhov@gmail.com> on 07.09.2021.
#  Copyright © 2021 Dmitrii Torkhov. All rights reserved.
#

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(lockfree_container
        VERSION 1.1.0
        LANGUAGES CXX)

##########
# Source #
##########

add_library(lockfree_container INTERFACE)
add_library(oo::lockfree_container ALIAS lockfree_container)

#

set(INCLUDE_DIR
        ${CMAKE_CURRENT_LIST_DIR}/include)

set(PUBLIC_HEADERS
        ${INCLUDE_DIR}/lockfree_container/lockfree_container.h)

target_include_directories(lockfree_container INTERFACE
        $<BUILD_INTERFACE:${INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include>)

###########
# Install #
###########

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-config
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}${INSTALL_SUFFIX}/${PROJECT_NAME})

install(FILES ${PUBLIC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}${INSTALL_SUFFIX}/${PROJECT_NAME})

install(EXPORT ${PROJECT_NAME}-config DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

###########
# Testing #
###########

enable_testing()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif ()