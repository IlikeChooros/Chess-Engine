#include <cengine/utils.h>

namespace chess
{

/**
 * @brief Parse the command line arguments, when '--help' is passed the program exits
 * @return A vector of parsed arguments
 */
ArgParser::arg_map_t ArgParser::parse()
{
    m_args.push_back({"--help", "", defaultValidator, "Show this help message"});
    for (int i = 1; i < m_argc; i++)
    {
        std::string arg = m_argv[i];
        if (arg[0] == '-')
        {
            // Check if the argument is valid
            auto it = std::find_if(m_args.begin(), m_args.end(), 
                [&arg](const arg_t& a) { return a.name == arg; });
            
            if (it != m_args.end())
            {
                // Check if the argument has a value
                if (i + 1 < m_argc && m_argv[i + 1][0] != '-')
                {
                    it->value = m_argv[++i];
                    if (!it->validator(it->value))
                    {
                        std::cerr << "Invalid value for argument " << arg << ": " << it->value << "\n";
                        exit(1);
                    }
                }
                // boolean argument
                else
                {
                    it->value = "true";
                }
            }
            else
            {
                std::cerr << "Unknown argument: " << arg << "\n";
            }
        }
    }

    // Print help message if --help is passed
    auto help_it = std::find_if(m_args.begin(), m_args.end(), 
        [](const arg_t& a) { return a.name == "--help"; });

    if (help_it != m_args.end() && !help_it->value.empty())
    {
        std::cout << "Usage: " << m_argv[0] << " [options]\n";
        std::cout << "Options:\n";
        
        for (const auto& arg : m_args)
        {
            std::cout << "  " << arg.name << ": " << arg.description 
                    << " (default="<< arg.value <<")" << "\n";
        }
        exit(0);
    }

    // Store the arguments in a map
    arg_map_t parsed_args;
    for (const auto& arg : m_args)
        if (!arg.value.empty())
            parsed_args[arg.name] = arg.value;
    
    return parsed_args;
}

} // namespace chess