#include "topic.hpp"
#include "toc.hpp"
#include "common_includes.hpp"
#include "help_map_parser.hpp"

using namespace std::filesystem;

namespace
{
auto sanitize_topic_filename(std::string input) -> std::string
{
    std::replace(input.begin(), input.end(), '.', '-');
    sanitize_filename(input, g_bad_chars, g_good_chars);
    input.erase(std::remove_if(input.begin(), input.end(), [](unsigned char c)
                               { return std::isspace(c) || c == '\n' || c == '\r'; }),
                input.end());
    return input + ".htm";
}
} // namespace

auto main(int argc, char **argv) -> int
{
    const path input_file = (argc > 1) ? path{argv[1]} : path{"../test_files/ExtraHelpObjectMap_Nanosecond.xml"};

    ParsedHelpMap parsed_help_map;
    std::string parse_error;
    if (!parse_help_map_xml(input_file, parsed_help_map, parse_error))
    {
        std::cerr << parse_error << '\n';
        return 1;
    }

    auto &domain_objects = parsed_help_map.domain_objects;
    auto &functions = parsed_help_map.functions;

    const std::vector<path> output_paths = {
        g_proj_dir,
        g_toc_dir,
        g_content_dir,
        g_autogen_dir,
        g_do_dir,
        g_cons_dir,
        g_props_dir,
        g_meths_dir,
        g_funcs_dir,
    };

    for (const auto &output_path : output_paths)
    {
        std::error_code ec;
        create_directories(output_path, ec);
        if (ec)
        {
            std::cerr << "Failed to create directory '" << output_path.string() << "': " << ec.message() << '\n';
            return 1;
        }
    }

    std::vector<Topic> topics;

    Topic available_objects("Available Objects", "available objects", "available_objects", TopicType::available_dos);
    if (!available_objects.create_topic("../../objects_and_functions.htm", sanitize_topic_filename(domain_objects.front().Name)))
    {
        return 1;
    }
    topics.push_back(available_objects);

    for (auto it = domain_objects.begin(); it != domain_objects.end(); ++it)
    {
        std::string prev_topic;
        std::string next_topic;

        if (it == domain_objects.begin())
        {
            prev_topic = "available_objects.htm";
        }
        else
        {
            prev_topic = sanitize_topic_filename(std::prev(it)->Name);
        }

        if (std::next(it) == domain_objects.end())
        {
            next_topic = sanitize_topic_filename(domain_objects.front().Name);
        }
        else
        {
            next_topic = sanitize_topic_filename(std::next(it)->Name);
        }

        Topic topic(it->Name, it->Name, it->Name, TopicType::domain_object);
        if (!topic.create_topic(prev_topic, next_topic))
        {
            return 1;
        }
        topics.push_back(topic);

        for (const auto &constructor_signature : it->Constructors)
        {
            Topic constructor_topic(constructor_signature, constructor_signature, constructor_signature, TopicType::constructor, true);
            if (!constructor_topic.create_topic())
            {
                return 1;
            }
        }

        for (const auto &property_name : it->Properties)
        {
            const auto display_name = it->Name + "." + property_name;
            Topic property_topic(display_name, property_name, it->Name + "-" + property_name, TopicType::property);
            if (!property_topic.create_topic())
            {
                return 1;
            }
        }

        for (const auto &[method_name, overloads] : it->Methods)
        {
            if (overloads.size() > 1U)
            {
                const auto method_name_with_object = it->Name + "." + method_name;
                Topic method_topic(method_name_with_object, method_name_with_object, method_name_with_object, TopicType::method);
                if (!method_topic.create_topic())
                {
                    return 1;
                }
            }

            for (const auto &overload_signature : overloads)
            {
                Topic overload_topic(overload_signature, overload_signature, overload_signature, TopicType::method, true);
                if (!overload_topic.create_topic())
                {
                    return 1;
                }
            }
        }
    }

    const std::string previous_for_available_functions = "../DomainObjects/" + sanitize_topic_filename(domain_objects.back().Name);
    const std::string next_for_available_functions = functions.empty()
                                                         ? "../../application_program_interface.htm"
                                                         : sanitize_topic_filename(functions.begin()->first);
    Topic available_functions("Available Functions", "available functions", "available_functions", TopicType::available_funcs);
    if (!available_functions.create_topic(previous_for_available_functions, next_for_available_functions))
    {
        return 1;
    }
    topics.push_back(available_functions);

    for (auto it = functions.begin(); it != functions.end(); ++it)
    {
        std::string prev_topic;
        std::string next_topic;

        if (it == functions.begin())
        {
            prev_topic = "available_functions.htm";
        }
        else
        {
            prev_topic = sanitize_topic_filename(std::prev(it)->first);
        }

        if (std::next(it) == functions.end())
        {
            next_topic = "../../application_program_interface.htm";
        }
        else
        {
            next_topic = sanitize_topic_filename(std::next(it)->first);
        }

        Topic function_topic(it->first, it->first, it->first, TopicType::function);
        if (!function_topic.create_topic(prev_topic, next_topic))
        {
            return 1;
        }

        for (const auto &overload_signature : it->second)
        {
            Topic overload_topic(overload_signature, overload_signature, overload_signature, TopicType::function, true);
            if (!overload_topic.create_topic())
            {
                return 1;
            }

            topics.push_back(overload_topic);
        }
    }

    TableOfContent toc(topics, "toc");
    if (!toc.create_toc())
    {
        std::cerr << "Failed to write TOC file.\n";
        return 1;
    }

    return 0;
}