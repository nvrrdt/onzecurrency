find_package(OpenSSL REQUIRED)

set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

add_library(common SHARED 
                     crypto_shab58.cpp
                     crypto_ecdsa.cpp
                     crypto_rsa.cpp
)

target_include_directories(common PUBLIC
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/build/external/include/ecdsa"
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/libcommon/crypto_shab58.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/crypto_ecdsa.hpp"
                "${PROJECT_SOURCE_DIR}/include/libcommon/crypto_rsa.hpp"
                )

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/libcommon/libcommon.so DESTINATION lib/onze)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze)
