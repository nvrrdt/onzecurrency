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

# enable_testing:

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
                      OpenSSL::SSL
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CURL_LIBRARIES}
)
gtest_discover_tests(tests)

# post_build: copy ip_peers.json to build directory
add_custom_command(
    TARGET crowd 
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy  
                     "${CMAKE_CURRENT_SOURCE_DIR}/ip_peers.json" 
                     "$<TARGET_FILE_DIR:crowd>/ip_peers.json"
        COMMAND ${CMAKE_COMMAND} -E copy  
                     "${CMAKE_CURRENT_SOURCE_DIR}/new_users_pool.json" 
                     "$<TARGET_FILE_DIR:crowd>/new_users_pool.json"
)