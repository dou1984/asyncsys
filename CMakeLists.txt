cmake_minimum_required(VERSION 3.14.0)

project(asyncsys)

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release)
#  SET(CMAKE_BUILD_TYPE Debug)
ENDIF()

aux_source_directory(./src SRC_DIR)

set(CMAKE_C_COMPILER g++)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "-ggdb -std=c++2a")

include_directories(/usr/local/include /usr/include/mysql)
link_directories(/usr/local/lib64/ /usr/local/lib/)

#add_executable(${PROJECT_NAME} ${SRC_DIR})
add_library(${PROJECT_NAME} STATIC ${SRC_DIR})
target_link_libraries(${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${PROJECT_NAME})
