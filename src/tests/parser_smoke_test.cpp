#include "help_map_parser.hpp"

auto main(int argc, char **argv) -> int
{
    if (argc < 2)
    {
        return 2;
    }

    ParsedHelpMap parsed{};
    std::string error_message;
    if (!parse_help_map_xml(argv[1], parsed, error_message))
    {
        return 1;
    }

    if (parsed.domain_objects.empty())
    {
        return 1;
    }

    if (parsed.domain_objects.front().Name.empty())
    {
        return 1;
    }

    return 0;
}
