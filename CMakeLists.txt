cmake_minimum_required(VERSION 3.6.0)
project(StrawberryData)

set (CMAKE_CXX_STANDARD 11)
add_definitions(-DROOT_DIR="${CMAKE_SOURCE_DIR}")

find_package(realsense2 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost 1.45.0 COMPONENTS filesystem REQUIRED)

add_executable(grabber "src/grabber.cpp")
target_include_directories(grabber PUBLIC include ${Boost_INCLUDE_DIRS})
target_link_libraries(grabber ${Boost_LIBRARIES} ${OpenCV_LIBS} realsense2)