add_executable(${PROJECT_NAME} main.cpp)


add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/utils $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils)

add_subdirectory(cengine)

target_link_libraries(${PROJECT_NAME} PUBLIC cengine) 