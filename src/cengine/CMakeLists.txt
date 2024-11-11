add_library(cengine)

target_sources(cengine
    PRIVATE
        src/board.cpp
        src/manager.cpp
        src/manager_impl.cpp
        src/test.cpp
        src/move_gen.cpp
        src/eval.cpp
        src/search.cpp
        src/hash.cpp
        src/threads.cpp
        src/move_ordering.cpp
        src/magic_bitboards.cpp
        src/uci.cpp
        src/log.cpp
        src/pgn.cpp
        src/settings.cpp
        src/engine.cpp
        src/perft.cpp
        src/mailbox.cpp
        src/zobrist.cpp
)

target_include_directories(cengine PUBLIC include)

target_link_libraries(cengine sfml-graphics sfml-window sfml-system)