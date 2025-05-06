#include <ui/analysis.hpp>


UI_NAMESPACE_BEGIN

void Analysis::loop(arg_map_t& args)
{
    M_loop_setup(args);
}

void Analysis::clear()
{
    BaseManager::clear();
}

void Analysis::add_args(chess::ArgParser& parser)
{
    // Add --fen, --limits
    M_add_base_game_options(parser);
}

void Analysis::M_render()
{
    // Empty implementation
}




void Analysis::M_process_param_input(chess::ArgParser::arg_map_t& map)
{
    // Check if --fen or --limits were specified
    M_process_game_options(map);
}

// chess::ArgParser::arg_map_t Analysis::M_process_arguments(int argc, char** argv)
// {
//     m_parser.setArgs(argc, argv);
//     // Adds --fen and --limits
//     M_add_base_game_options();
//     return m_parser.parse();
// }

UI_NAMESPACE_END