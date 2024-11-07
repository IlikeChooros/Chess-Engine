cmake_minimum_required(VERSION 3.4)

project(CEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CENGINE_TEST_BUILD OFF CACHE BOOL "Build tests")

if (CENGINE_TEST_BUILD)
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
endif()

add_subdirectory(src)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

install(TARGETS ${PROJECT_NAME})


# Testing if CENGINE_TEST_BUILD is ON
if (CENGINE_TEST_BUILD)
  include(CTest)

  # Tests
  enable_testing()
  add_subdirectory(tests)
endif()
