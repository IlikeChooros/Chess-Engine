add_library(ui)

target_sources(ui 
PRIVATE
    "src/renderer.cpp"
    "src/game_manager.cpp"
    "src/base_manager.cpp"
    "src/single_player.cpp"
    "src/analysis.cpp"
)

target_include_directories(ui PUBLIC include)
target_link_libraries(ui PUBLIC cengine)