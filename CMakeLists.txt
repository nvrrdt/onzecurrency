cmake_minimum_required(VERSION 3.10)

project(onzehub-terminal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

add_executable(onzehub-terminal ${all_SRCS})
target_link_libraries(${PROJECT_NAME}
        /usr/local/lib/onzehub/liblogin.so
)

### enable_testing:

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
find_package(GMock REQUIRED)
include_directories(${GMOCK_INCLUDE_DIRS})

file(GLOB all_TESTS "${PROJECT_SOURCE_DIR}/tests/*.cpp")

add_executable(tests ${all_TESTS})

target_link_libraries(tests 
                      GTest::GTest
                      GTest::Main
                      ${GMOCK_BOTH_LIBRARIES}
)
gtest_discover_tests(tests)
