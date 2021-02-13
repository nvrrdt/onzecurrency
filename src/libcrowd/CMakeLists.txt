find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

add_library(crowd SHARED
                     configdir.cpp
                     block.cpp
                     interface.cpp
                     ip_peers.cpp
                     p2p_client.cpp
                     p2p_server.cpp
                     p2p.cpp
                     poco.cpp
                     protocol.cpp
                     prev_hash.cpp
                     upnp.cpp
                     verification.cpp
)

target_include_directories(crowd PUBLIC
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp-prefix/src/cryptopp"
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/liblogin/auth.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/configdir.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/interface.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/ip_peers.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/json.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/merkle_tree.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p_message.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/poco.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/random.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/prev_hash.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/verification.hpp"
                )

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/libcrowd/libcrowd.so DESTINATION lib/onze)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze)
