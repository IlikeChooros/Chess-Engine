#pragma once

#include "base_manager.hpp"

UI_NAMESPACE_BEGIN

class Analysis : public ChessManager
{
public:
    Analysis() = default;
    void loop(int argc = 0, char** argv = nullptr) override;
    void clear() override;
private:

    void M_init() override;
    void M_render();
    void M_process_param_input(int argc = 0, char** argv = nullptr);
    chess::ArgParser::arg_map_t M_process_arguments(int argc, char** argv);
};

UI_NAMESPACE_END