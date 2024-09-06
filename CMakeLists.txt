cmake_minimum_required(VERSION 3.5.0)
project(space-terrain-tiler VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 20)

add_executable(space-terrain-tiler main.cpp)

set(BOOST_ROOT "/home/mark/dev/work/scout/main/terrain-builder.git/main/tblibs/boost_1_86_0")
set(GDAL_DIR "/home/mark/dev/work/scout/main/terrain-builder.git/main/tblibs/gdal-3.9.2")
# set(Boost_USE_STATIC_LIBS ON)

find_package(Boost 1.86.0 COMPONENTS program_options REQUIRED)
find_package(GDAL REQUIRED)
find_package(PROJ REQUIRED)

target_link_libraries(space-terrain-tiler Boost::program_options)
target_link_libraries(space-terrain-tiler GDAL::GDAL)
target_link_libraries(space-terrain-tiler PROJ::proj)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
