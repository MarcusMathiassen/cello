cmake_minimum_required(VERSION 3.17.0_1)
project(cello)

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_STANDARD 17)

#Load headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/)

#Load sources
file(GLOB SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/main.mm
  )
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/*.metal

#message("-- CXX ${CMAKE_CXX_COMPILER}")
message("-- Generated with build type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter -Wno-unused-function )

#DNDEBUG -fprofile-generate -fprofile-use -fsanitize=address
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Wno-unused-function -Wno-unused-parameter -g -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

message("-- Debug   flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message("-- Release flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message("-- Generator set to: ${CMAKE_GENERATOR}")

# RESOURCE FILE LOCATION
file(GLOB RESOURCE_FILES
"${CMAKE_CURRENT_SOURCE_DIR}/src/*.metal"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${RESOURCE_FILES})

# Features required
target_compile_features(${PROJECT_NAME} PRIVATE cxx_lambda_init_captures)

# BUNDLE OPTIONS
set_target_properties(${PROJECT_NAME} PROPERTIES
MACOSX_BUNDLE false
MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Resources/info.plist
#MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Resources/icon_16.png
MACOSX_FRAMEWORK_IDENTIFIER org.cmake.${PROJECT_NAME}
RESOURCE "${RESOURCE_FILES}"
)

# Get Freetype
find_package(FREETYPE 2.9 REQUIRED)
if(FREETYPE_FOUND)
    include_directories(${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})
endif()

# Get GLFW
find_package(glfw3 3.3 REQUIRED)
if (GLFW_FOUND)
    include_directories(${GLFW_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})
endif()

# LINK TO TARGET
target_link_libraries(${PROJECT_NAME}
  glfw
  "-framework Cocoa"
  "-framework IOKit"
  "-framework CoreVideo"
  "-framework Foundation"
  "-framework AppKit"
  "-framework Metal"
  "-framework QuartzCore"
  )
