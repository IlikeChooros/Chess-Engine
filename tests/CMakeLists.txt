project(CEngineTests LANGUAGES CXX)

file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

add_executable(CEngineTests ${TEST_SOURCES})

target_link_libraries(CEngineTests PUBLIC GTest::gtest GTest::gtest_main cengine)


if(WIN32)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<BOOL:${ARCH_64BITS}>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>
        VERBATIM)
endif()

include(GoogleTest)
gtest_discover_tests(CEngineTests)


