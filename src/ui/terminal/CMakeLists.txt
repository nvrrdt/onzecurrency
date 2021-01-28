find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Threads)

add_executable(onze-terminal main.cpp)

add_dependencies(onze-terminal ecdsa++)

target_include_directories(onze-terminal PUBLIC
                          "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                          "${PROJECT_SOURCE_DIR}/include/liblogin"
                          "${PROJECT_SOURCE_DIR}/include/libcrowd"
                          "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa"
                          )

target_link_libraries(onze-terminal
        LINK_PUBLIC login
        LINK_PUBLIC crowd
        OpenSSL::SSL
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        "/usr/lib/librocksdb.so"
        "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
        "/usr/lib/x86_64-linux-gnu/libsnappy.a"
        libecdsa++.so
)

#${PROJECT_SOURCE_DIR}/build/external/lib/libecdsa++.so
# include_directories(
#         ${PROJECT_SOURCE_DIR}/include/ui/terminal
#         ${PROJECT_SOURCE_DIR}/include/libcrowd
#         ${PROJECT_SOURCE_DIR}/include/liblogin
#         ${PROJECT_SOURCE_DIR}/src/ui/terminal
#         ${CURL_INCLUDE_DIR}
# )

# link_directories(${OPENSSL_LIBRARIES})

# file(GLOB all_SRCS
#         "${PROJECT_SOURCE_DIR}/include/ui/terminal/*.hpp"
#         "${PROJECT_SOURCE_DIR}/src/ui/terminal/*.cpp"
# )

# file(GLOB all_HEADERS
#         "${PROJECT_SOURCE_DIR}/include/**.hpp"
# )

# SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/onzehub")

# add_executable(onzehub-terminal "${all_SRCS}")
# set_target_properties(onzehub-terminal PROPERTIES PUBLIC_HEADER "${all_HEADERS}")
# target_link_libraries(onzehub-terminal
#         LINK_PUBLIC login
#         LINK_PUBLIC crowd
#         OpenSSL::SSL
#         ${Boost_FILESYSTEM_LIBRARY}
#         ${Boost_SYSTEM_LIBRARY}
#         ${CMAKE_THREAD_LIBS_INIT}
#         "/usr/lib/librocksdb.so"
#         "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
#         "/usr/lib/x86_64-linux-gnu/libsnappy.a"
#         ${CURL_LIBRARIES}
#         ${PROJECT_SOURCE_DIR}/build/3rd_party/ecdsa_cxx/libecdsa++.so
# )

### install:

install(FILES ${PROJECT_SOURCE_DIR}/build/src/ui/terminal/onze-terminal DESTINATION bin)


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
