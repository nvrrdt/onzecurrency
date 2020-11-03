find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/libcrowd
        ${PROJECT_SOURCE_DIR}/src/libcrowd
        ${Boost_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIR}
        "/usr/include/miniupnpc"
)
link_directories(${OPENSSL_LIBRARIES})

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/libcrowd/*.cpp"
)

file(GLOB all_HEADERS
        "${PROJECT_SOURCE_DIR}/include/libcrowd/*.hpp"
)

add_library(crowd SHARED ${all_SRCS})
set_target_properties(crowd PROPERTIES PUBLIC_HEADER "${all_HEADERS}")
target_include_directories(crowd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

### install:

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/libcrowd/libcrowd.so DESTINATION lib/onzehub)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onzehub)


### enable_testing

include(CTest)

file(GLOB all_TESTS "${PROJECT_SOURCE_DIR}/tests/libcrowd/*.cpp")
add_executable(tests ${all_TESTS} ${all_SRCS})
target_compile_definitions(tests PUBLIC BOOST_TEST_DYN_LINK)
target_link_libraries(tests
                      OpenSSL::SSL
                      ${Boost_FILESYSTEM_LIBRARY}
                      ${Boost_SYSTEM_LIBRARY}
                      ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${CURL_LIBRARIES}
                      "/usr/lib/librocksdb.so"
                      "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
                      "/usr/lib/x86_64-linux-gnu/libsnappy.a"
)

add_test(NAME tests COMMAND tests)
enable_testing()