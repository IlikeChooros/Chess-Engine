add_library(cengine)

target_sources(cengine
    PRIVATE
        src/board.cpp
        src/input_handle.cpp
        src/manager.cpp
        src/manager_impl.cpp
        src/test.cpp
        src/move_gen.cpp
        src/eval.cpp
        src/search.cpp
        src/hash.cpp
        src/threads.cpp
        src/move_ordering.cpp
        src/ui.cpp
        src/uci.cpp
)

target_include_directories(cengine PUBLIC include)

target_link_libraries(cengine sfml-graphics sfml-window sfml-system)