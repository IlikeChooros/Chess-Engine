cmake_minimum_required(VERSION 3.14)

project(CEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Optionally set the build type if not specified externally
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

# Set GNU compiler flags for debug and release
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
   # Debug: no optimizations, include debug symbols
   set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
   # Release: optimize for speed with full optimizations
   set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
   # MSVC debug flags and release flags (adjust as needed)
   set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od")
   set(CMAKE_CXX_FLAGS_RELEASE "/O2")
endif()



# Download gtest
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()
FetchContent_MakeAvailable(googletest)

# Load the CEngine library
add_subdirectory(src)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

install(TARGETS ${PROJECT_NAME})


# Testing if CENGINE_TEST_BUILD is ON
include(CTest)

# Tests
enable_testing()
add_subdirectory(tests)

