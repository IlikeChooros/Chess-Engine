add_library(ui)

target_sources(ui 
PRIVATE
    "src/renderer.cpp"
    "src/game_manager.cpp"
)

target_include_directories(ui PUBLIC include)
target_link_libraries(ui PUBLIC cengine)