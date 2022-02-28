set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

link_directories(${GTKMM_LIBRARY_DIRS})

add_library(crowd SHARED
                     configdir.cpp
                     block.cpp
                     ip_peers.cpp
                     p2p_client.cpp
                     p2p_server.cpp
                     p2p.cpp
                     rocksy.cpp
                     protocol.cpp
                     prev_hash.cpp
                     upnp.cpp
                     verification.cpp
                     full_hash.cpp
)

target_include_directories(crowd PUBLIC
                        ${GTKMM_INCLUDE_DIRS}
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/ui/desktop"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/include/libcoin"
                        "${PROJECT_SOURCE_DIR}/include/libpoco"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp"
                        "${PROJECT_SOURCE_DIR}/include/plog"
                        ${ENET_INCLUDE_DIRS}
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/liblogin/auth.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/configdir.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/ip_peers.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/merkle_tree.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p_message.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/rocksy.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/random.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/prev_hash.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/verification.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p_network.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/full_hash.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/p2p_network_c.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/protocol_c.hpp"
                )

INSTALL(TARGETS crowd ARCHIVE DESTINATION lib/onze COMPONENT libraries)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze COMPONENT headers)
