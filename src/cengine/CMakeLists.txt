add_library(cengine)

target_sources(cengine
    PRIVATE
        src/board.cpp
        src/input_handle.cpp
        src/manager.cpp
        src/ui.cpp
)

target_include_directories(cengine PUBLIC include)

target_link_libraries(cengine sfml-graphics sfml-window sfml-system)