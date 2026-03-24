
#include "topic.hpp"
#include "toc.hpp"
#include "common_includes.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

auto main() -> int
{
    // proof-of-concept, ingest the ExtraHelpObjectMap_Nanosecond.xml file
    std::vector<DomainObject> domainObjects;
    std::unordered_map<std::string, std::vector<std::string>> funcs;

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

    auto isExcludedType = [](const std::string& type) -> bool {
        std::vector<std::string> excluded_types = {"FFDiagnostics"};

        if (type.find("List<") != std::string::npos) {
            return (type.find("List<Object>") == std::string::npos);
        }

        for (auto t : excluded_types) {
            if (type.find(t) != std::string::npos) {
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
                std::unordered_map<std::string, std::vector<std::string>> meths;
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
                        else {
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
    for (XMLElement *e = functionsNode->FirstChildElement(); e != nullptr; e = e->NextSiblingElement()) {
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
    for (auto p : autogen_paths) {
        std::error_code ec;
        if (!std::filesystem::create_directory(p, ec)) {
            if (ec) {
                std::cout << ec.message();
                return 1;
            }
        }
    }

    // 6. Now iterate through each of the DomainObjects, create a Topic, and generate the placeholder file
    std::vector<Topic> topics;
    for (auto d : domainObjects)
    {
        auto tName = d.Name;
        auto tKeyword = d.Name;
        auto tFilename = d.Name;
        Topic t(tName, tKeyword, tFilename, TopicType::domain_object);
        if (!t.create_topic())
            return 1;

        topics.push_back(t);

        if (!d.Constructors.empty())
        {
            for (auto c : d.Constructors)
            {
                auto name = c;
                auto keyword = c;
                auto filename = c;
                Topic cT(name, keyword, filename, TopicType::constructor);
                if (!cT.create_topic())
                    return 1;
            }
        }

        if (!d.Properties.empty())
        {
            for (auto p : d.Properties)
            {
                auto name = d.Name + "." + p;
                auto keyword = p;
                auto filename = d.Name + "-" + p;
                Topic pT(name, keyword, filename, TopicType::property);
                if (!pT.create_topic())
                    return 1;
            }
        }

        if (!d.Methods.empty())
        {
            for (auto m : d.Methods)
            {
                for (auto o : m.second)
                {
                    auto name = o;
                    auto keyword = o;
                    auto filename = o;
                    Topic mT(name, keyword, filename, TopicType::method);
                    if (!mT.create_topic())
                        return 1;
                }
            }
        }
    }

    // 7. Next iterate through each of the Functions, create a Topic, and generate the placeholder file
    for (auto f : funcs) {
        for (auto o : f.second) {
            auto name = o;
            auto keyword = o;
            auto filename = o;
            Topic fT(name, keyword, filename, TopicType::function);
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