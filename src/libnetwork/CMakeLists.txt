set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

link_directories(${GTKMM_LIBRARY_DIRS})

add_library(network SHARED
                     p2p_client.cpp
                     p2p_server.cpp
)

target_include_directories(network PUBLIC
                        ${GTKMM_INCLUDE_DIRS}
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/ui/desktop"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/include/libcoin"
                        "${PROJECT_SOURCE_DIR}/include/libpoco"
                        "${PROJECT_SOURCE_DIR}/include/libnetwork"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp"
                        "${PROJECT_SOURCE_DIR}/include/plog"
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/libnetwork/p2p_network.hpp"
                "${PROJECT_SOURCE_DIR}/include/libnetwork/p2p_message.hpp"
                )

INSTALL(TARGETS network ARCHIVE DESTINATION lib/onze COMPONENT libraries)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze COMPONENT headers)
