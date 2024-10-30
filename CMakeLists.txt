# Build file
# Copyright (C) 2024 polarbub

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as published by
# the Free Software Foundation

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.28)

# Name the project
project(npmapStitch)
enable_language(CXX)

set(CMAKE_BUILD_TYPE "Debug")
#set(CMAKE_BUILD_TYPE "Release")

#optimization level
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
#warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-comment -Wall")

# Set C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#Include project headers
include_directories("${CMAKE_SOURCE_DIR}/src")

find_package(CURL REQUIRED)
find_package(OpenCV REQUIRED)

#Remove header from SRC_FILES regex:
#' src/[^\n ]*\.h'
# Define a variable containing a list of source files for the project
set(SRC_FILES
         src/main.cpp
    )

# Define the build target for the executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS} )

target_include_directories(${PROJECT_NAME} PUBLIC ${CURL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${OpenCV_INCLUDE_DIRS})
