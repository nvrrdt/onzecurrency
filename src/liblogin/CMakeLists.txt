include_directories(
        ${PROJECT_SOURCE_DIR}/include/liblogin
        ${PROJECT_SOURCE_DIR}/include/libcrowd
        ${PROJECT_SOURCE_DIR}/include/ui/terminal
        ${PROJECT_SOURCE_DIR}/src/liblogin
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/liblogin/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/liblogin/*.cpp"
)

file(GLOB all_HEADERS
        "${PROJECT_SOURCE_DIR}/include/liblogin/*.hpp"
)

add_library(login SHARED "${all_SRCS}")
set_target_properties(login PROPERTIES PUBLIC_HEADER "${all_HEADERS}")
target_include_directories(login PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

### install:

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/liblogin/liblogin.so DESTINATION lib/onzehub)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onzehub)


### enable_testing:

#enable_testing()
#find_package(GTest REQUIRED)
#include(GoogleTest)

#set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
#find_package(GMock REQUIRED)
#include_directories(${GMOCK_INCLUDE_DIRS})

#file(GLOB all_TESTS "${PROJECT_SOURCE_DIR}/tests/*.cpp")

#add_executable(tests ${all_TESTS})

#target_link_libraries(tests 
#                      GTest::GTest
#                      GTest::Main
#                      ${GMOCK_BOTH_LIBRARIES}
#)
#gtest_discover_tests(tests)