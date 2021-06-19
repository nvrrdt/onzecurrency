set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

add_library(coin SHARED
                     p2p_c.cpp
                     p2p_server_c.cpp
                     p2p_client_c.cpp
                     transactions.cpp
                     poco_c.cpp
                     prev_hash_c.cpp
                     protocol_c.cpp
                     block_c.cpp
                     block_matrix.cpp
                     verification_c.cpp
)

target_include_directories(coin PUBLIC
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/include/libcoin"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp"
                        ${ENET_INCLUDE_DIRS}
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/libcommon/json.hpp"
                "${PROJECT_SOURCE_DIR}/include/liblogin/auth.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/configdir.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/ip_peers.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/merkle_tree.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p_message.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/rocksy.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/random.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/prev_hash.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/verification.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/poco.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/message_vec.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/p2p_network.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/all_hashes.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcrowd/full_hash.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/p2p_network_c.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/transactions.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/poco_c.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/prev_hash_c.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/protocol_c.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/merkle_tree_c.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/block_matrix.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcoin/verification_c.hpp"
                )

INSTALL(TARGETS coin ARCHIVE DESTINATION lib/onze COMPONENT libraries)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze COMPONENT headers)
