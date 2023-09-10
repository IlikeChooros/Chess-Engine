add_library(src ui.cpp board.cpp manager.cpp)
add_subdirectory(img)
target_link_libraries(src sfml-graphics sfml-window sfml-system)
