add_executable(${PROJECT_NAME} main.cpp)

# Custom commands to copy font and img directories
add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/font $<TARGET_FILE_DIR:${PROJECT_NAME}>/font)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/img $<TARGET_FILE_DIR:${PROJECT_NAME}>/img)

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/utils $<TARGET_FILE_DIR:${PROJECT_NAME}>/utils)

add_subdirectory(cengine)
add_subdirectory(ui)


target_link_libraries(${PROJECT_NAME} PUBLIC cengine ui) 