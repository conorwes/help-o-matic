
#include "topic.hpp"
#include "toc.hpp"
#include "common_includes.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

auto main() -> int
{
    // proof-of-concept, ingest the ExtraHelpObjectMap_Nanosecond.xml file
    std::vector<DomainObject> domainObjects;
    std::map<std::string, std::vector<std::string>, CaseInsensitiveComparer> funcs;

    XMLDocument doc;

    // 1. Load the file
    auto filepath = std::string("C:\\help-o-matic\\test_files\\ExtraHelpObjectMap_Nanosecond.xml");
    XMLError eResult = doc.LoadFile(filepath.c_str());
    if (eResult != XML_SUCCESS)
    {
        std::cerr << "Error loading XML: " << doc.ErrorIDToName(eResult) << std::endl;
        return 1;
    }

    // 2. Find the root element
    XMLNode *root = doc.FirstChildElement();
    if (root == nullptr)
        return 1;

    auto isExcludedType = [](const std::string &type) -> bool
    {
        std::vector<std::string> excluded_types = {"FFDiagnostics"};

        if (type.find("List<") != std::string::npos)
        {
            return (type.find("List<Object>") == std::string::npos);
        }

        for (auto t : excluded_types)
        {
            if (type.find(t) != std::string::npos)
            {
                return true;
            }
        }

        return false;
    };

    // 3. Recursive search for "Object" tags
    XMLNode *objectsNode = root->FirstChildElement();
    for (XMLElement *e = objectsNode->FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
    {
        if (e != nullptr)
        {
            // We should only ever hit "Object" nodes, but it can't hurt to be sure
            if (std::string(e->Value()) == std::string("Object"))
            {
                // 1. First get the Object's name
                const char *nameAttr = e->Attribute("name");
                auto nameStr = (nameAttr != nullptr && !isExcludedType(std::string(nameAttr))) ? std::string(nameAttr) : "INVALID";
                if (nameStr == "INVALID")
                    continue;

                DomainObject d;
                d.Name = nameStr;

                // 2. Next iterate through all children and retrieve Constructors, Properties, and Methods
                std::vector<std::string> consts;
                std::vector<std::string> props;
                std::map<std::string, std::vector<std::string>, CaseInsensitiveComparer> meths;
                for (XMLElement *m = e->FirstChildElement(); m != nullptr; m = m->NextSiblingElement())
                {
                    // Properties don't have any children
                    if (m->FirstChildElement() == nullptr)
                    {
                        const char *memNameAttr = m->Attribute("name");
                        if (memNameAttr != nullptr)
                        {
                            props.push_back(std::string(memNameAttr));
                        }
                    }
                    else
                    {
                        // Constructors and Methods have children
                        if (std::string(m->Value()) == std::string("Constructors"))
                        {
                            // This node is the Constructors node
                            for (XMLElement *c = m->FirstChildElement(); c != nullptr; c = c->NextSiblingElement())
                            {
                                const char *sig = c->GetText();
                                consts.push_back(std::string(sig));
                            }
                        }
                        else if (std::string(m->Value()) == std::string("Member"))
                        {
                            // This node is a Method
                            const char *methNameAttr = m->Attribute("name");
                            if (methNameAttr != nullptr)
                            {
                                XMLElement *overloads = m->FirstChildElement();
                                for (XMLElement *o = overloads->FirstChildElement(); o != nullptr; o = o->NextSiblingElement())
                                {
                                    const char *signature = o->GetText();
                                    meths[methNameAttr].push_back(std::string(signature));
                                }
                            }
                        }
                        else
                        {
                            return 1;
                        }
                    }
                }

                if (consts.size() != 0)
                {
                    d.Constructors = consts;
                }
                d.Properties = props;
                d.Methods = meths;
                domainObjects.push_back(d);
            }
        }
    }

    // 4. Recursive search for "Function" tags
    XMLNode *functionsNode = objectsNode->NextSibling();
    for (XMLElement *e = functionsNode->FirstChildElement(); e != nullptr; e = e->NextSiblingElement())
    {
        // We should only ever hit "Function" nodes, but it can't hurt to be sure
        if (std::string(e->Value()) == std::string("Function"))
        {
            // This node is a Function
            const char *funcNameAttr = e->Attribute("name");
            if (funcNameAttr != nullptr)
            {
                XMLElement *overloads = e->FirstChildElement();
                for (XMLElement *o = overloads->FirstChildElement(); o != nullptr; o = o->NextSiblingElement())
                {
                    const char *signature = o->GetText();
                    funcs[funcNameAttr].push_back(std::string(signature));
                }
            }
        }
    }

    // 5. Verify the existence of the output directories
    std::vector<std::filesystem::path> autogen_paths = {g_autogen_dir, g_do_dir, g_cons_dir, g_props_dir, g_meths_dir, g_funcs_dir};
    for (auto p : autogen_paths)
    {
        std::error_code ec;
        if (!std::filesystem::create_directory(p, ec))
        {
            if (ec)
            {
                std::cout << ec.message();
                return 1;
            }
        }
    }

    // 6. Now iterate through each of the DomainObjects, create a Topic, and generate the placeholder file
    std::vector<Topic> topics;
    auto availableObjects = Topic(std::string("Available Objects"), std::string("available objects"), std::string("available_objects"), TopicType::available_dos);
    availableObjects.create_topic("../../objects_and_functions.htm", domainObjects.begin()->Name + ".htm");
    topics.push_back(availableObjects);

    for (auto it = domainObjects.begin(); it != domainObjects.end(); ++it)
    {
        std::string prev_topic, next_topic;
        if (it == domainObjects.begin())
        {
            prev_topic = "available_objects.htm";
        }
        else
        {
            auto filename = std::prev(it)->Name;
            sanitize_filename(filename, g_bad_chars, g_good_chars);
            prev_topic = filename + ".htm";
        }

        if (std::next(it) == domainObjects.end())
        {
            // loop back to first DomainObject
            next_topic = domainObjects.begin()->Name + ".htm";
        }
        else
        {
            auto filename = std::next(it)->Name;
            sanitize_filename(filename, g_bad_chars, g_good_chars);
            next_topic = filename + ".htm";
        }

        auto tName = it->Name;
        auto tKeyword = it->Name;
        auto tFilename = it->Name;
        Topic t(tName, tKeyword, tFilename, TopicType::domain_object);
        if (!t.create_topic(prev_topic, next_topic))
            return 1;

        topics.push_back(t);

        if (!it->Constructors.empty())
        {
            for (auto c : it->Constructors)
            {
                auto name = c;
                auto keyword = c;
                auto filename = c;
                Topic cT(name, keyword, filename, TopicType::constructor, true);
                if (!cT.create_topic())
                    return 1;
            }
        }

        if (!it->Properties.empty())
        {
            for (auto p : it->Properties)
            {
                auto name = it->Name + "." + p;
                auto keyword = p;
                auto filename = it->Name + "-" + p;
                Topic pT(name, keyword, filename, TopicType::property);
                if (!pT.create_topic())
                    return 1;
            }
        }

        if (!it->Methods.empty())
        {
            for (auto m : it->Methods)
            {
                if (m.second.size() > 1)
                {
                    // Multiple overloads -> we need a method summary page
                    auto name = it->Name + "." + m.first;
                    auto keyword = it->Name + "." + m.first;
                    auto filename = it->Name + "." + m.first;
                    Topic mT(name, keyword, filename, TopicType::method);
                    if (!mT.create_topic())
                        return 1;
                }

                for (auto o : m.second)
                {
                    auto name = o;
                    auto keyword = o;
                    auto filename = o;
                    Topic mT(name, keyword, filename, TopicType::method, true);
                    if (!mT.create_topic())
                        return 1;
                }
            }
        }
    }

    // 7. Next iterate through each of the Functions, create a Topic, and generate the placeholder file
    auto availableFunctions = Topic(std::string("Available Functions"), std::string("available functions"), std::string("available_functions"), TopicType::available_funcs);
    availableFunctions.create_topic(domainObjects.back().Name + ".htm", funcs.begin()->first + ".htm");
    topics.push_back(availableFunctions);

    for (auto it = funcs.begin(); it != funcs.end(); ++it)
    {
        std::string prev_topic, next_topic;
        if (it == funcs.begin())
        {
            prev_topic = "available_functions.htm";
        }
        else
        {
            auto filename = std::prev(it)->first;
            sanitize_filename(filename, g_bad_chars, g_good_chars);
            prev_topic = filename + ".htm";
        }

        if (std::next(it) == funcs.end())
        {
            next_topic = "../../application_program_interface.htm";
        }
        else
        {
            auto filename = std::next(it)->first;
            sanitize_filename(filename, g_bad_chars, g_good_chars);
            next_topic = filename + ".htm";
        }

        // First make a Function summary page
        auto name = it->first;
        auto keyword = it->first;
        auto filename = it->first;
        Topic fT(name, keyword, filename, TopicType::function);
        if (!fT.create_topic(prev_topic, next_topic))
            return 1;

        // Then make pages for each individual overload
        for (auto o : it->second)
        {
            auto name = o;
            auto keyword = o;
            auto filename = o;
            Topic fT(name, keyword, filename, TopicType::function, true);
            if (!fT.create_topic())
                return 1;

            topics.push_back(fT);
        }
    }

    // 8. Finally, create a TableOfContent and generate the file
    TableOfContent toc(topics, "toc");
    if (!toc.create_toc())
        return 1;

    return 0;
}