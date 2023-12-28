# FindYamlCpp.cmake
# Find yaml-cpp library and include files

# Check if CMake version is at least 3.12
if(${CMAKE_VERSION} VERSION_LESS "3.12")
    message(FATAL_ERROR "CMake version must be at least 3.12 to use FindYamlCpp.cmake")
endif()

# Look for yaml-cpp library
find_library(YAML_CPP_LIBRARY
    NAMES yaml-cpp
    HINTS /usr/local/lib /usr/lib
)

# Look for yaml-cpp include directory
find_path(YAML_CPP_INCLUDE_DIR
    NAMES yaml-cpp/yaml.h
    HINTS /usr/local/include /usr/include
)

# Check if yaml-cpp library and include directory were found
if(NOT YAML_CPP_LIBRARY OR NOT YAML_CPP_INCLUDE_DIR)
    message(FATAL_ERROR "yaml-cpp library or include directory not found")
endif()

# Check if we have built shared libraries
set(YAML_CPP_SHARED_LIBS_BUILT OFF)
if(TARGET yaml-cpp::yaml-cpp)
    set(YAML_CPP_SHARED_LIBS_BUILT ON)
endif()

# Set the variables
set(YAML_CPP_LIBRARIES ${YAML_CPP_LIBRARY})
set(YAML_CPP_LIBRARY_DIR /usr/local/lib /usr/lib) # Set appropriate library directories
set(YAML_CPP_INCLUDE_DIR ${YAML_CPP_INCLUDE_DIR})

# Provide information to the user
message(STATUS "Found yaml-cpp library: ${YAML_CPP_LIBRARY}")
message(STATUS "Found yaml-cpp include directory: ${YAML_CPP_INCLUDE_DIR}")
message(STATUS "Built shared libraries: ${YAML_CPP_SHARED_LIBS_BUILT}")

# Export the variables to the cache
set(YAML_CPP_LIBRARIES ${YAML_CPP_LIBRARIES} CACHE STRING "Libraries to link against")
set(YAML_CPP_LIBRARY_DIR ${YAML_CPP_LIBRARY_DIR} CACHE STRING "Directory containing libraries")
set(YAML_CPP_SHARED_LIBS_BUILT ${YAML_CPP_SHARED_LIBS_BUILT} CACHE BOOL "Whether we have built shared libraries or not")
set(YAML_CPP_INCLUDE_DIR ${YAML_CPP_INCLUDE_DIR} CACHE PATH "Include directory")

# Mark the variables for export
if(NOT TARGET yaml-cpp::yaml-cpp)
    set(YAML_CPP_LIBRARIES ${YAML_CPP_LIBRARY} CACHE STRING "Libraries to link against" FORCE)
    set(YAML_CPP_LIBRARY_DIR ${YAML_CPP_LIBRARY_DIR} CACHE STRING "Directory containing libraries" FORCE)
    set(YAML_CPP_SHARED_LIBS_BUILT ${YAML_CPP_SHARED_LIBS_BUILT} CACHE BOOL "Whether we have built shared libraries or not" FORCE)
    set(YAML_CPP_INCLUDE_DIR ${YAML_CPP_INCLUDE_DIR} CACHE PATH "Include directory" FORCE)
endif()
