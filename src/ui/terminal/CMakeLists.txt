set(BOOST_INCLUDEDIR /usr/include)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Threads)

include_directories(
        ${PROJECT_SOURCE_DIR}/include/
        ${PROJECT_SOURCE_DIR}/src/ui/terminal
)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/ui/terminal/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/ui/terminal/*.cpp"
)

file(GLOB all_HEADERS
        "${PROJECT_SOURCE_DIR}/include/ui/terminal/*"
)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_executable(onzehub-terminal "${all_SRCS}")
target_link_libraries(onzehub-terminal
        #/usr/local/lib/onzehub/liblogin.so # TODO: make this also work on windows
        LINK_PUBLIC crowd
        #/usr/local/lib/onzehub/libcrowd.so
        LINK_PUBLIC login
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        ${CMAKE_THREAD_LIBS_INIT}
)
#INSTALL(TARGETS onzehub-terminal
#        RUNTIME DESTINATION bin
#        COMPONENT runtime
#)

### enable_testing:

#enable_testing()
#find_package(GTest REQUIRED)
#include(GoogleTest)

#set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
#find_package(GMock REQUIRED)
#include_directories(${GMOCK_INCLUDE_DIRS})

#file(GLOB all_TESTS "${PROJECT_SOURCE_DIR}/tests/*.cpp")

#add_executable(tests ${all_TESTS})

#target_link_libraries(tests 
#                      GTest::GTest
#                      GTest::Main
#                      ${GMOCK_BOTH_LIBRARIES}
#)
#gtest_discover_tests(tests)
