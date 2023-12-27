cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

project(kapua CXX C ASM)

# load extra modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include_directories (
	${CMAKE_SOURCE_DIR}/src
	${YAML_CPP_INCLUDE_DIR}
)

add_library(
	kapua SHARED 

	src/Config.cpp
	src/Logger.cpp
	src/Node.cpp
	src/Protocol.cpp
	src/Core.cpp
	src/UDPNetwork.cpp
	src/DistributedBlockStore.cpp
)

find_package(LIBYAML REQUIRED)
include_directories(${LIBYAML_INCLUDE_DIR})
target_link_libraries(kapua ${LIBYAML_LIBRARIES})

find_package(CURL REQUIRED) 
include_directories(${CURL_INCLUDE_DIR})
target_link_libraries(kapua ${CURL_LIBRARIES})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Examples

project(kapua_link CXX C ASM)

add_executable(
	kapua_link
	examples/kapua_link/main.cpp
)
target_link_libraries(kapua_link kapua)