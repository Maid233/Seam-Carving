cmake_minimum_required(VERSION 3.17)
set(CMAKE_CXX_STANDARD 11)
project(SeamCarver VERSION 0.0.0)

message(${PROJECT_SOURCE_DIR})

configure_file(header_config.h.in header_config.h)

#opencv package
option(OpenCV_STATIC OFF)
find_package(OpenCV REQUIRED
        PATHS 3Party/opencv
        )
include_directories(3Party/opencv/include)
message(STATUS "OpenCV found")

include_directories(include)
include_directories(src)
file(GLOB SRC_CPP
        src/*.cpp)

list(APPEND SRC_CPP
        main.cpp
        )

add_executable(SeamCarver ${SRC_CPP})


# find config.h
target_include_directories(SeamCarver PUBLIC
        "${PROJECT_BINARY_DIR}"
        )

target_link_libraries(SeamCarver
        ${OpenCV_LIBS}
        )