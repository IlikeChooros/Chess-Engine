#pragma once

#include "namespaces.hpp"
#include "base_manager.hpp"


UI_NAMESPACE_BEGIN

class SinglePlayer : public ChessManager
{
public:
    SinglePlayer() = default;
    void loop(arg_map_t&) override;
    void clear() override;

    static void add_args(chess::ArgParser& parser);
private:

    void M_render() override;
    void M_process_param_input(arg_map_t& map) override;

    void M_engine_move();
    void M_player_move();
    void M_render_gamesummary();

    bool m_player_side{false};
};


UI_NAMESPACE_END