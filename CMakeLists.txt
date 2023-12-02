cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(kapua CXX C ASM)

include_directories (${CMAKE_SOURCE_DIR}/src)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(
	kapua

	src/core.cpp
	src/logger.cpp
)

# Examples

project(kapua_link CXX C ASM)

add_executable(
	kapua_link
	examples/kapua_link/main.cpp
)
target_link_libraries(kapua_link kapua)