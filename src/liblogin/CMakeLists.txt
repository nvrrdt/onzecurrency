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
                        "${PROJECT_SOURCE_DIR}/include/libpoco"
                        "${PROJECT_SOURCE_DIR}/build/cryptopp"
                        "${PROJECT_SOURCE_DIR}/include/plog"
                          )

### install:

set(all_HEADERS "${PROJECT_SOURCE_DIR}/include/liblogin/auth.hpp")

INSTALL(TARGETS login ARCHIVE DESTINATION lib/onze COMPONENT libraries)
INSTALL(FILES ${all_HEADERS} DESTINATION include/onze COMPONENT headers)
