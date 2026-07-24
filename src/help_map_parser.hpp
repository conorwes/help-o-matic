#pragma once

#include "common_includes.hpp"

struct ParsedHelpMap
{
    std::vector<DomainObject> domain_objects;
    std::map<std::string, std::vector<std::string>, CaseInsensitiveComparer> functions;
};

auto parse_help_map_xml(const std::filesystem::path &input_file, ParsedHelpMap &output, std::string &error_message) -> bool;
