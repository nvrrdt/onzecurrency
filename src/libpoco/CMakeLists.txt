set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

add_library(poco SHARED
                     poco_crowd.cpp
                     poco_coin.cpp
                     all_hashes.cpp
                     message_vec.cpp
                     transactions.cpp
                     block_matrix.cpp
                     block_matrix_c.cpp
)

target_include_directories(poco PUBLIC
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/include/libcoin"
                        "${PROJECT_SOURCE_DIR}/include/libpoco"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp"
                        ${ENET_INCLUDE_DIRS}
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/libcommon/json.hpp"
                "${PROJECT_SOURCE_DIR}/include/libpoco/poco_crowd.hpp"
                "${PROJECT_SOURCE_DIR}/include/libpoco/poco_coin.hpp"
                "${PROJECT_SOURCE_DIR}/include/libpoco/all_hashes.hpp"
                "${PROJECT_SOURCE_DIR}/include/libpoco/message_vec.hpp"
                "${PROJECT_SOURCE_DIR}/include/libpoco/transactions.hpp"
                "${PROJECT_SOURCE_DIR}/include/libpoco/block_matrix.hpp"
                )

INSTALL(TARGETS poco ARCHIVE DESTINATION lib/onze COMPONENT libraries)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze COMPONENT headers)
