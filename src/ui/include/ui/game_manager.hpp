#pragma once

#include "single_player.hpp"
#include "analysis.hpp"

UI_NAMESPACE_BEGIN

class GameManager : public BaseManager
{
public:
    GameManager() = default;
    void loop(int argc = 0, char** argv = nullptr);

    void loop(arg_map_t&) override {} // empty
private:
    typedef std::unique_ptr<BaseManager> manager_t;

    void M_init();
    void M_render() override {} // empty
    void M_process_param_input(arg_map_t& map) override;
    
    chess::ArgParser m_parser;
    manager_t m_manager{nullptr};
};


UI_NAMESPACE_END