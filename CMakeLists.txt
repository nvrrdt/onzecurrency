cmake_minimum_required(VERSION 3.13)

project(onzecurrency)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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