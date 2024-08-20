cmake_minimum_required(VERSION 3.4)

project(CEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wpedantic -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-value -Wno-unused-local-typedefs -Wno-unused-private-field")
# set(CMAKE_CXX_FLAGS "-Wall -pg")
# set(CMAKE_EXE_LINKER_FLAGS "-pg")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Download SFML & gtest
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

add_subdirectory(src)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

install(TARGETS ${PROJECT_NAME})

include(CTest)

# Tests
enable_testing()
add_subdirectory(tests)
