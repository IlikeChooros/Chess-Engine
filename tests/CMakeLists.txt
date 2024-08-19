project(CEngineTests LANGUAGES CXX)

file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_executable(CEngineTests ${TEST_SOURCES})

target_link_libraries(CEngineTests PUBLIC GTest::gtest GTest::gtest_main cengine)

include(GoogleTest)
gtest_discover_tests(CEngineTests)


