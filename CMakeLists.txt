cmake_minimum_required(VERSION 3.10)

# set the project name
project(crowd)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Threads)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIR}
)
link_directories(${OPENSSL_LIBRARIES})

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB all_HEADERS
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
)

add_library(${PROJECT_NAME} SHARED ${all_SRCS})
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${all_HEADERS}")
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE src)

INSTALL(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION lib/onzehub
        LIBRARY DESTINATION lib/onzehub
        PUBLIC_HEADER DESTINATION include/onzehub
        COMPONENT library
)

### enable CPack

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nico Verrijdt") #required

INCLUDE(CPack)

### enable_testing

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

# post_build: copy ip_adress_master_peer.json to build directory
add_custom_command(
    TARGET ${PROJECT_NAME} 
    POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy  
                     "${CMAKE_CURRENT_SOURCE_DIR}/ip_adress_master_peer.json" 
                     "$<TARGET_FILE_DIR:crowd>/ip_adress_master_peer.json"
        COMMAND ${CMAKE_COMMAND} -E copy  
                     "${CMAKE_CURRENT_SOURCE_DIR}/new_users_pool.json" 
                     "$<TARGET_FILE_DIR:crowd>/new_users_pool.json"
)


### print all variables

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

#print_all_variables()