#include <cengine/utils.h>

namespace chess
{

/**
 * @brief Check if the required arguments are set, and push specified ones 
 * (without unspecified optional args)
 */
void ArgParser::ParsedArguments::process(const arg_list_t& args)
{
    m_args.clear();

    // Go through the arguments and check if they are set
    for (const auto& arg : args)
    {
        if (arg.props.required && !arg.props.set)
        {
            std::cerr << "Required argument not set: " << arg.flags[0] << "\n";
            exit(1);
        }

        // Push required elements and default ones 
        // (omit not specified optional arguments)
        if (arg.props.set || !arg.props.default_value.empty())
            m_args.push_back(arg);
    }
}

/**
 * @brief Add an argument to the parser with a list of flags
 * @param flags A list of possible flags for the argument (e.g., {"--fen", "-f"})
 * @param props The properties of the argument: required, help message, 
 * default value, type, validator, action
 * @throws std::runtime_error if the default value is not valid
 */
void ArgParser::add_argument(
    const std::vector<std::string>& flags, const arg_props_t& props)
{
    // Check if the flags weren't already added
    for (const auto& flag : flags)
    {
        // Check if the flag exists in the list
        if (M_find(m_args, flag) != m_args.end())
            throw std::runtime_error("Flag already exists: " + flag);
    }

    arg_value_props_t value_props(props);

    // Set the default value, check if it matches the type
    if (!props.default_value.empty())
    {
        if (props.type == INT) {
            // Try to convert the default value to an int
            try { value_props.value = std::stoi(props.default_value); }
            catch (...) { throw std::runtime_error("Invalid default value for " + flags[0]); }
        }
        else if (props.type == BOOL) {
            // For BOOL values, check the defaults, user should properly use 'action' for this
            // to set the value (for example, if the flag is passed, set it to true, without checking the value)
            if (props.default_value == "true" || props.default_value == "1") {
                value_props.value = true;
            } else if (props.default_value == "false" || props.default_value == "0") {
                value_props.value = false;
            } else {
                value_props.value = false; // Default to false if default_value is not explicitly true/1
            }
        }
        // STRING or ANY
        else {
            value_props.value = props.default_value;
        }
    }

    // Match validator by type (if not set)
    if (props.validator == nullptr)
    {
        if (props.type == INT)
            value_props.validator = intValidator;
        else if (props.type == BOOL)
            value_props.validator = booleanValidator;
        else if (props.type == STRING)
            value_props.validator = stringValidator;
        else 
            value_props.validator = defaultValidator;
    }

    if (props.action == nullptr)
        value_props.action = store;

    
    m_args.push_back({flags, value_props});
}

/**
 * @brief Parse the command line arguments, when '--help' is passed the program exits
 * @return A vector of parsed arguments
 */
ArgParser::arg_map_t ArgParser::parse()
{
    // Step 1. Prase the command line arguments, calls actions, and validators
    for (int i = 1; i < m_argc; i++)
    {
        std::string arg = m_argv[i];

        // Check if that's a flag
        if (arg[0] != '-')
            continue;

        // Check if the flag is valid (exists in the list)
        auto it = std::find_if(m_args.begin(), m_args.end(), 
            [&arg](const new_arg_t& a) { 
                // Check if the flag is in the list of flags
                return std::find(a.flags.begin(), a.flags.end(), arg) != a.flags.end(); 
        });

        // Unknown flag
        if (it == m_args.end()) {
            std::cerr << "Unknown argument: " << arg << "\n";
            continue;
        }

        // Check the value (if any) with validator
        std::string value = "";

        // Parse until another flag is found
        while (i + 1 < m_argc && m_argv[i + 1][0] != '-')
        {
            value += std::string(m_argv[++i]) + " ";
        }

        // Remove the trailing space
        if (!value.empty())
            value.pop_back();

        // Check if the value if valid
        if (!it->props.validator(value))
        {
            std::cerr << "Invalid value for argument " << arg << ": " << value << "\n";
            exit(1);
        }

        // Store the value in the argument
        it->props.set = true;
        it->props.action(it->props.value, value);
    }

    // Step 2. Check if the help message is passed
    auto help_it = m_args.begin(); // help is always the first argument
    if (help_it->props.set)
    {
        // Action will print the help message
        exit(0);
    }

    // Step 3. Store the arguments in an object
    ParsedArguments parsed_args(m_args);
    parsed_args.process(m_args);
    return parsed_args;
}

} // namespace chess