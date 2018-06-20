cmake_minimum_required(VERSION 3.6.0)
project(StrawberryData)

set (CMAKE_CXX_STANDARD 17)
add_definitions(-DROOT_DIR="${CMAKE_SOURCE_DIR}")

find_package(Threads)
find_package(realsense2 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost 1.45.0 COMPONENTS filesystem REQUIRED)

file(GLOB SRC_FILES "src/*.cpp")
file(GLOB HEADER_FILES "include/*.hpp")
file(GLOB THIRD_PARTY_HEADER_FILES "third-party/*.hpp")

set(DEPENDANCIES ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${OpenCV_LIBS} realsense2)
set(INCLUDE_DIRECTORIES include third-party ${Boost_INCLUDE_DIRS})

add_executable(grabber ${SRC_FILES})
target_include_directories(grabber PUBLIC ${INCLUDE_DIRECTORIES})
target_link_libraries(grabber ${DEPENDANCIES})