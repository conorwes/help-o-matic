#include "help_map_parser.hpp"

namespace
{
auto is_excluded_type(const std::string &type) -> bool
{
    static const std::vector<std::string> excluded_types = {"FFDiagnostics"};

    if (type.find("List<") != std::string::npos)
    {
        return type.find("List<Object>") == std::string::npos;
    }

    for (const auto &excluded_type : excluded_types)
    {
        if (type.find(excluded_type) != std::string::npos)
        {
            return true;
        }
    }

    return false;
}
} // namespace

auto parse_help_map_xml(const std::filesystem::path &input_file, ParsedHelpMap &output, std::string &error_message) -> bool
{
    output = {};
    error_message.clear();

    tinyxml2::XMLDocument doc;
    const auto load_result = doc.LoadFile(input_file.string().c_str());
    if (load_result != tinyxml2::XML_SUCCESS)
    {
        error_message = "Error loading XML file '" + input_file.string() + "': " + doc.ErrorIDToName(load_result);
        return false;
    }

    tinyxml2::XMLNode *const root = doc.FirstChildElement();
    if (root == nullptr)
    {
        error_message = "XML does not have a root element.";
        return false;
    }

    tinyxml2::XMLNode *const objects_node = root->FirstChildElement();
    if (objects_node == nullptr)
    {
        error_message = "Could not find objects section in XML.";
        return false;
    }

    for (tinyxml2::XMLElement *object_element = objects_node->FirstChildElement(); object_element != nullptr; object_element = object_element->NextSiblingElement())
    {
        if (std::string_view{object_element->Value()} != "Object")
        {
            continue;
        }

        const char *name_attr = object_element->Attribute("name");
        if (name_attr == nullptr || is_excluded_type(name_attr))
        {
            continue;
        }

        DomainObject domain_object{};
        domain_object.Name = name_attr;

        for (tinyxml2::XMLElement *member_element = object_element->FirstChildElement(); member_element != nullptr; member_element = member_element->NextSiblingElement())
        {
            if (member_element->FirstChildElement() == nullptr)
            {
                if (const char *member_name_attr = member_element->Attribute("name"); member_name_attr != nullptr)
                {
                    domain_object.Properties.emplace_back(member_name_attr);
                }
                continue;
            }

            if (std::string_view{member_element->Value()} == "Constructors")
            {
                for (tinyxml2::XMLElement *constructor_element = member_element->FirstChildElement(); constructor_element != nullptr; constructor_element = constructor_element->NextSiblingElement())
                {
                    if (const char *signature = constructor_element->GetText(); signature != nullptr)
                    {
                        domain_object.Constructors.emplace_back(signature);
                    }
                }
            }
            else if (std::string_view{member_element->Value()} == "Member")
            {
                const char *method_name_attr = member_element->Attribute("name");
                if (method_name_attr == nullptr)
                {
                    continue;
                }

                if (tinyxml2::XMLElement *overloads = member_element->FirstChildElement(); overloads != nullptr)
                {
                    for (tinyxml2::XMLElement *overload = overloads->FirstChildElement(); overload != nullptr; overload = overload->NextSiblingElement())
                    {
                        if (const char *signature = overload->GetText(); signature != nullptr)
                        {
                            domain_object.Methods[method_name_attr].emplace_back(signature);
                        }
                    }
                }
            }
            else
            {
                error_message = "Unexpected object member element: '" + std::string(member_element->Value()) + "'.";
                return false;
            }
        }

        output.domain_objects.push_back(std::move(domain_object));
    }

    tinyxml2::XMLNode *const functions_node = objects_node->NextSibling();
    if (functions_node == nullptr)
    {
        error_message = "Could not find functions section in XML.";
        return false;
    }

    for (tinyxml2::XMLElement *function_element = functions_node->FirstChildElement(); function_element != nullptr; function_element = function_element->NextSiblingElement())
    {
        if (std::string_view{function_element->Value()} != "Function")
        {
            continue;
        }

        const char *function_name_attr = function_element->Attribute("name");
        if (function_name_attr == nullptr)
        {
            continue;
        }

        if (tinyxml2::XMLElement *overloads = function_element->FirstChildElement(); overloads != nullptr)
        {
            for (tinyxml2::XMLElement *overload = overloads->FirstChildElement(); overload != nullptr; overload = overload->NextSiblingElement())
            {
                if (const char *signature = overload->GetText(); signature != nullptr)
                {
                    output.functions[function_name_attr].emplace_back(signature);
                }
            }
        }
    }

    if (output.domain_objects.empty())
    {
        error_message = "No domain objects were discovered in the XML map.";
        return false;
    }

    return true;
}
