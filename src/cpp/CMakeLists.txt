cmake_minimum_required(VERSION 3.31)
project(si4735-cpp)
set(CMAKE_CXX_STANDARD 17)
enable_testing()

set(SI4735_LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SI4735_LIB_SOURCE_DIR ${SI4735_LIB_DIR}/src)

file(GLOB_RECURSE SI4735_FILES ${SI4735_LIB_SOURCE_DIR}/*.cpp)
message(STATUS "SI4735 source dir: ${SI4735_LIB_SOURCE_DIR}")
message(STATUS "SI4735_FILES found: ${SI4735_FILES}")

add_library(si4735-cpp STATIC ${SI4735_FILES})

target_include_directories(si4735-cpp PUBLIC
  ${SI4735_LIB_SOURCE_DIR}
  ${FLUX_CPP}/src
)
target_link_libraries(si4735-cpp PRIVATE flux-cpp)

if(ENABLE_COVERAGE AND (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CX_COMPILER_ID MATCHES "Clang"))
  target_compile_options(si4735-cpp PRIVATE --coverage -O0 -g)
  target_link_options(si4735-cpp PRIVATE --coverage)
endif()