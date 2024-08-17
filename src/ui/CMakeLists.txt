add_library(ui)

target_sources(ui
    PRIVATE
        src/ui.cpp
        src/input_handle.cpp
)

target_include_directories(ui PUBLIC include)

target_link_libraries(ui sfml-graphics sfml-window sfml-system cengine)