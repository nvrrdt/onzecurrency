cmake_minimum_required(VERSION 3.10)

# set the project name
project(crowd)

set(CMAKE_CXX_STANDARD 14)

find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${CURL_INCLUDE_DIR}
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(crowd ${all_SRCS})
target_link_libraries(${PROJECT_NAME}
                      OpenSSL::SSL
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CURL_LIBRARIES}
)