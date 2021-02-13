find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Threads)

find_package(CryptoPP REQUIRED PATHS "${PROJECT_SOURCE_DIR}/build/cryptopp-prefix/src/cryptopp")

add_executable(onze-terminal main.cpp)

target_include_directories(onze-terminal PUBLIC
                          "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                          "${PROJECT_SOURCE_DIR}/include/libcommon"
                          "${PROJECT_SOURCE_DIR}/include/liblogin"
                          "${PROJECT_SOURCE_DIR}/include/libcrowd"
                          "${PROJECT_SOURCE_DIR}/build/cryptopp-prefix/src/cryptopp"
                          )

target_link_libraries(onze-terminal
        LINK_PUBLIC login
        LINK_PUBLIC crowd
        LINK_PUBLIC common
        cryptopp-static
        OpenSSL::SSL
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        "/usr/lib/librocksdb.so"
        "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
        "/usr/lib/x86_64-linux-gnu/libsnappy.a"
)

### install:

install(FILES ${PROJECT_SOURCE_DIR}/build/src/ui/terminal/onze-terminal DESTINATION bin)
