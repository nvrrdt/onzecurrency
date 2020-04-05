cmake_minimum_required(VERSION 3.10)

project(login)

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

file(GLOB all_HEADERS
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

add_library(login ${all_SRCS})
set_target_properties(login PROPERTIES PUBLIC_HEADER ${all_HEADERS})
INSTALL(TARGETS login
        ARCHIVE DESTINATION lib/onzehub
        LIBRARY DESTINATION lib/onzehub
        PUBLIC_HEADER DESTINATION include/onzehub
        COMPONENT library
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