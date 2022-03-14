# CMAKE extensions
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

find_package(CryptoPP REQUIRED PATHS "${PROJECT_SOURCE_DIR}/build/cryptopp")

add_executable(tests_crowd main.cpp
                           configdir_unittest.cpp
                           p2p_unittest.cpp
                           rocksy_unittest.cpp
                           protocol_unittest.cpp
                           verification_unittest.cpp
)

target_include_directories(tests_crowd PUBLIC
                          "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                          "${PROJECT_SOURCE_DIR}/include/libcommon"
                          "${PROJECT_SOURCE_DIR}/include/liblogin"
                          "${PROJECT_SOURCE_DIR}/include/libcrowd"
                          "${PROJECT_SOURCE_DIR}/include/libcoin"
                          "${PROJECT_SOURCE_DIR}/include/libpoco"
                          "${PROJECT_SOURCE_DIR}/include/libnetwork"
                          "${PROJECT_SOURCE_DIR}/build/cryptopp"
                          "${PROJECT_SOURCE_DIR}/include/plog"
                          )

target_link_libraries(tests_crowd
        LINK_PUBLIC crowd
        LINK_PUBLIC login
        LINK_PUBLIC coin
        LINK_PUBLIC common
        LINK_PUBLIC poco
        LINK_PUBLIC network
        cryptopp-static
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        "/usr/lib/librocksdb.so"
        "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
        "/usr/lib/x86_64-linux-gnu/libsnappy.a"
)
