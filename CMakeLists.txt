cmake_minimum_required(VERSION 3.13)

# set the project name
project(onzecurrency)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory (src/libcrowd)
add_subdirectory (src/liblogin)
add_subdirectory (src/ui/terminal)

### enable CPack

find_package (Threads)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Nico Verrijdt") #required

INCLUDE(CPack)

# post_build: copy ip_adress_master_peer.json to build directory
#add_custom_command(
#    TARGET ${PROJECT_NAME} 
#    POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy  
#                     "${CMAKE_CURRENT_SOURCE_DIR}/ip_adress_master_peer.json" 
#                     "$<TARGET_FILE_DIR:crowd>/ip_adress_master_peer.json"
#        COMMAND ${CMAKE_COMMAND} -E copy  
#                     "${CMAKE_CURRENT_SOURCE_DIR}/new_users_pool.json" 
#                     "$<TARGET_FILE_DIR:crowd>/new_users_pool.json"
#)


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