cmake_minimum_required(VERSION 3.5.0)
project(space-terrain-tiler VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(STT_VERSION_MAJOR 0)
set(STT_VERSION_MINOR 1)
set(STT_VERSION_PATCH 0)
set(TERRAIN_TILE_SIZE 65)
set(TERRAIN_MASK_SIZE 256)

add_executable(space-terrain-tiler main.cpp)

set(BOOST_ROOT "/home/mark/dev/work/scout/main/terrain-builder.git/main/tblibs/boost_1_86_0")
set(GDAL_DIR "/home/mark/dev/work/scout/main/terrain-builder.git/main/tblibs/gdal-3.9.2")
# set(Boost_USE_STATIC_LIBS ON)
set(TOOL_TARGETS stt ${CMAKE_THREAD_LIBS_INIT})
message("tool targets: ${TOOL_TARGETS}")

find_package(Boost 1.86.0 COMPONENTS program_options REQUIRED)
find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)

add_library(ctb SHARED
    GlobalGeodetic.cpp
)

target_link_libraries(space-terrain-tiler Boost::program_options)
target_link_libraries(space-terrain-tiler GDAL::GDAL)
target_link_libraries(space-terrain-tiler PROJ::proj)
target_link_libraries(space-terrain-tiler ctb)

configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)
# include_directories("${PROJECT_SOURCE_DIR}")
# message("source dirs: ${PROJECT_SOURCE_DIR}")
# message("cmake dirs: ${CMAKE_SOURCE_DIR}")



set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wextra --pedantic")

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
