set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem unit_test_framework REQUIRED)

find_package(Threads)

add_library(login SHARED 
                     auth.cpp
)

target_include_directories(login PUBLIC
                        "${PROJECT_SOURCE_DIR}/include/ui/terminal"
                        "${PROJECT_SOURCE_DIR}/include/libcommon"
                        "${PROJECT_SOURCE_DIR}/include/liblogin"
                        "${PROJECT_SOURCE_DIR}/include/libcrowd"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp-prefix/src/cryptopp"
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/liblogin/auth.hpp")

INSTALL(FILES ${PROJECT_SOURCE_DIR}/build/src/liblogin/liblogin.so DESTINATION lib/onze)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze)
