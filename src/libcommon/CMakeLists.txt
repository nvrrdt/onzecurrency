set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

add_library(common SHARED 
                     crypto.cpp
                     base58.cpp
                     bech32.cpp
                     strencodings.cpp
)



target_include_directories(common PUBLIC
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/include/libcoin"
                        "${PROJECT_SOURCE_DIR}/include/libpoco"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp"
                        "${PROJECT_SOURCE_DIR}/include/plog"
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/libcommon/crypto.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/base58.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/bech32.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/attributes.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/span.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/strencodings.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/string.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/tinyformat.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/json.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/globals.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/print_or_log.hpp"
                )

INSTALL(TARGETS common ARCHIVE DESTINATION lib/onze COMPONENT libraries)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze COMPONENT headers)
