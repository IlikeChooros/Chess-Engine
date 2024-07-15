add_library(src ui.cpp board.cpp manager.cpp input_handle.cpp)
target_link_libraries(src sfml-graphics sfml-window sfml-system)
