cmake_minimum_required(VERSION 3.10)

# set the project name
project(crowd)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenSSL REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        )

add_executable(crowd ${all_SRCS})
target_link_libraries(${PROJECT_NAME} OpenSSL::SSL)