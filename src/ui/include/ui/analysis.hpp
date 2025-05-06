#pragma once

#include "base_manager.hpp"

UI_NAMESPACE_BEGIN

class Analysis : public ChessManager
{
public:
    Analysis() = default;
    void loop(chess::ArgParser::arg_map_t&) override;
    void clear() override;

    static void add_args(chess::ArgParser& parser);
private:

    void M_render();
    void M_process_param_input(arg_map_t& map) override;
};

UI_NAMESPACE_END