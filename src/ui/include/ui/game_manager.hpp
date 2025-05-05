#pragma once

#include "single_player.hpp"
#include "analysis.hpp"

UI_NAMESPACE_BEGIN

class GameManager : public BaseManager
{
public:
    GameManager() = default;
    void loop(int argc = 0, char** argv = nullptr);

private:
    typedef std::unique_ptr<BaseManager> manager_t;

    void M_init() override;
    void M_render() {} // empty
    void M_process_param_input(int argc = 0, char** argv = nullptr);
    chess::ArgParser::arg_map_t M_process_arguments(int argc = 0, char** argv = nullptr);
    
    manager_t m_manager{nullptr};
};


UI_NAMESPACE_END