# CMAKE extensions
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem REQUIRED)

find_package(Threads)

find_package(CryptoPP REQUIRED PATHS "${PROJECT_SOURCE_DIR}/build/cryptopp")
find_package(ENet REQUIRED)

add_executable(onze-terminal main.cpp)

target_include_directories(onze-terminal PUBLIC
                          "${PROJECT_SOURCE_DIR}/include/plog"
                          "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                          "${PROJECT_SOURCE_DIR}/include/libcommon"
                          "${PROJECT_SOURCE_DIR}/include/liblogin"
                          "${PROJECT_SOURCE_DIR}/include/libcrowd"
                          "${PROJECT_SOURCE_DIR}/include/libcoin"
                          "${PROJECT_SOURCE_DIR}/include/libpoco"
                          "${PROJECT_SOURCE_DIR}/build/cryptopp"
                          
                          ${ENET_INCLUDE_DIRS}
                          )

target_link_libraries(onze-terminal
        LINK_PUBLIC login
        LINK_PUBLIC crowd
        LINK_PUBLIC common
        LINK_PUBLIC coin
        LINK_PUBLIC poco
        cryptopp-static
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
        "/usr/lib/librocksdb.so"
        "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
        "/usr/lib/x86_64-linux-gnu/libsnappy.a"
        ${ENET_LIBRARIES}
)

### install:

install(TARGETS onze-terminal
        RUNTIME
        PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
        DESTINATION bin
        COMPONENT applications)
