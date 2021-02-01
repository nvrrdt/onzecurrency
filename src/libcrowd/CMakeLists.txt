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
                        "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa"
                          )



# include_directories(
#         ${PROJECT_SOURCE_DIR}/include/libcommon
#         ${PROJECT_SOURCE_DIR}/include/libcrowd
#         ${PROJECT_SOURCE_DIR}/include/liblogin
#         ${PROJECT_SOURCE_DIR}/include/ui/terminal
#         ${PROJECT_SOURCE_DIR}/src/libcrowd
#         ${PROJECT_SOURCE_DIR}/src/libcrowd/p2p
#         ${PROJECT_SOURCE_DIR}/3rd_party/ecdsa_cxx/src
#         ${Boost_INCLUDE_DIRS}
#         ${CURL_INCLUDE_DIR}
#         "/usr/include/miniupnpc"
# )
# link_directories(${OPENSSL_LIBRARIES})

# file(GLOB all_SRCS
#         "${PROJECT_SOURCE_DIR}/src/libcrowd/*.cpp"
#         "${PROJECT_SOURCE_DIR}/src/libcrowd/p2p/*.cpp"
# )

# file(GLOB all_HEADERS
#         "${PROJECT_SOURCE_DIR}/include/libcrowd/*.hpp"
# )

# add_library(crowd SHARED ${all_SRCS})
# set_target_properties(crowd PROPERTIES PUBLIC_HEADER "${all_HEADERS}")
# target_include_directories(crowd PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/ui/terminal/main.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/crypto_shab58.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/crypto_ecdsa.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/crypto_rsa.hpp"
                "${PROJECT_SOURCE_DIR}/include/liblogin/auth.hpp"
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
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/base58.h"
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/key.h"
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/pub_key.h"
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/rnd_man.h"
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/rnd_openssl.h"
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/rnd_os.h"
                "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa/sysutils.h"
                )

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/libcrowd/libcrowd.so DESTINATION lib/onze)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze)



# ### enable_testing

# include(CTest)

# file(GLOB all_TESTS "${PROJECT_SOURCE_DIR}/tests/libcrowd/*.cpp")
# add_executable(testscrowd ${all_TESTS} ${all_SRCS})
# target_compile_definitions(testscrowd PUBLIC BOOST_TEST_DYN_LINK)
# target_link_libraries(testscrowd
#                       LINK_PUBLIC crowd
#                       LINK_PUBLIC login
#                       LINK_PUBLIC ecdsa++
#                       OpenSSL::SSL
#                       ${Boost_FILESYSTEM_LIBRARY}
#                       ${Boost_SYSTEM_LIBRARY}
#                       ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
#                       ${CMAKE_THREAD_LIBS_INIT}
#                       ${CURL_LIBRARIES}
#                       "/usr/lib/librocksdb.so"
#                       "/usr/lib/x86_64-linux-gnu/libminiupnpc.a"
#                       "/usr/lib/x86_64-linux-gnu/libsnappy.a"
# )

# add_test(NAME testscrowd COMMAND testscrowd)
# enable_testing()