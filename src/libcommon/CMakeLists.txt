find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/libcommon
        ${PROJECT_SOURCE_DIR}/src/libcommon
        ${Boost_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIR}
)
link_directories(${OPENSSL_LIBRARIES})

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/libcommon/*.cpp"
)

file(GLOB all_HEADERS
        "${PROJECT_SOURCE_DIR}/include/libcommon/*.hpp"
)

add_library(common SHARED ${all_SRCS})
set_target_properties(common PROPERTIES PUBLIC_HEADER "${all_HEADERS}")
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

### install:

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/libcommon/libcommon.so DESTINATION lib/onzehub)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onzehub)


### enable_testing

# include(CTest)

# file(GLOB all_TESTS "${PROJECT_SOURCE_DIR}/tests/libcommon/*.cpp")
# add_executable(testscommon ${all_TESTS} ${all_SRCS})
# target_compile_definitions(testscommon PUBLIC BOOST_TEST_DYN_LINK)
# target_link_libraries(testscommon
#                       OpenSSL::SSL
#                       ${Boost_FILESYSTEM_LIBRARY}
#                       ${Boost_SYSTEM_LIBRARY}
#                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
#                       ${CMAKE_THREAD_LIBS_INIT}
#                       ${CURL_LIBRARIES}
# )

# add_test(NAME testscommon COMMAND testscommon)
# enable_testing()