#pragma once

#include "namespaces.hpp"
#include "base_manager.hpp"


UI_NAMESPACE_BEGIN

class SinglePlayer : public ChessManager
{
public:
    SinglePlayer() = default;
    void loop(int argc = 0, char** argv = nullptr) override;
    void clear() override;
private:
    typedef std::function<void(std::string)> callback_t;

    void M_render() override;
    void M_process_param_input(int argc = 0, char** argv = nullptr) override;
    chess::ArgParser::arg_map_t M_process_arguments(int argc, char** argv) override;
    void M_engine_move();
    void M_player_move();
    void M_render_gamesummary();

    bool m_player_side{false};
};


UI_NAMESPACE_END