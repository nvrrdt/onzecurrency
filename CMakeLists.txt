cmake_minimum_required(VERSION 3.13)

project(onzecurrency)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
ExternalProject_Add(cryptopp
                    GIT_REPOSITORY    https://github.com/weidai11/cryptopp.git
                    BUILD_IN_SOURCE 1
                    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
                    CONFIGURE_COMMAND wget -O CMakeLists.txt https://raw.githubusercontent.com/noloader/cryptopp-cmake/master/CMakeLists.txt && wget -O cryptopp-config.cmake https://raw.githubusercontent.com/noloader/cryptopp-cmake/master/cryptopp-config.cmake && cmake .
                    BUILD_COMMAND make
                    INSTALL_COMMAND ""
)

include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

add_subdirectory(src/ui/terminal)
add_subdirectory(src/libcommon)
add_subdirectory(src/libcrowd)
add_subdirectory(src/liblogin)
add_subdirectory(tests/ui/terminal)
add_subdirectory(tests/libcommon)
add_subdirectory(tests/libcrowd)
add_subdirectory(tests/liblogin)
add_subdirectory(tests/system_testing)

### enable CTest

enable_testing ()
add_test (NAME Test COMMAND Test)

### enable CPack

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nico Verrijdt") #required

INCLUDE(CPack)

### print all variables

macro(print_all_variables)
    message(STATUS "print_all_variables------------------------------------------{")
    get_cmake_property(_variableNames VARIABLES)
    foreach (_variableName ${_variableNames})
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "print_all_variables------------------------------------------}")
endmacro()

#print_all_variables()