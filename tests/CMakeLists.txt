project(CEngineTests LANGUAGES CXX)

add_executable(CEngineTests src/main.cc)

target_link_libraries(CEngineTests PUBLIC GTest::gtest GTest::gtest_main src)

gtest_discover_tests(CEngineTests)

