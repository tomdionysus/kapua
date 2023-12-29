cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(kapua CXX C ASM)

# load extra modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Includes
include_directories (
	${CMAKE_SOURCE_DIR}/src
)

file(GLOB SOURCES src/*.cpp)

add_library(
	kapua SHARED 
	${SOURCES}
)

find_package(YamlCpp REQUIRED)
include_directories(${YAML_CPP_INCLUDE_DIR})
target_link_libraries(kapua ${YAML_CPP_LIBRARIES})

find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(kapua ${CURL_LIBRARIES})

# Examples
project(kapua_link CXX C ASM)

add_executable(
	kapua_link
	examples/kapua_link/main.cpp
)
target_link_libraries(kapua_link kapua)

# Testing
enable_testing()

file(GLOB TEST_SOURCES test/*.cpp)

add_executable(
  kapua_test
	${TEST_SOURCES}
)
target_link_libraries(
  kapua_test
  kapua
  GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(kapua_test)
