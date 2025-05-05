#include <ui/analysis.hpp>


UI_NAMESPACE_BEGIN

void Analysis::loop(int argc, char** argv)
{
    M_loop_setup(argc, argv);
}

void Analysis::M_init()
{
    BaseManager::M_init();

    // Add the default argument
    m_parser.addArg("--analysis", "true", 
        chess::ArgParser::defaultValidator, 
            "Enable the single player mode (default: true)");
}

void Analysis::M_render()
{
    // Empty implementation
}

void Analysis::clear()
{
    BaseManager::clear();
}

void Analysis::M_process_param_input(int argc, char** argv)
{
    // Process the command line arguments
    auto args = M_process_arguments(argc, argv);

    // Check if --fen or --limits were specified
    M_process_game_options(args);
}

chess::ArgParser::arg_map_t Analysis::M_process_arguments(int argc, char** argv)
{
    m_parser.setArgs(argc, argv);
    // Adds --fen and --limits
    M_add_base_game_options();
    return m_parser.parse();
}

UI_NAMESPACE_END