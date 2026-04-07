#include "topic.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

Topic::Topic(std::string topic_name, std::string keyword, std::string filename, TopicType type, bool needs_signature)
    : m_topic_name(std::move(topic_name)), m_keyword(std::move(keyword)), m_type(type), m_needs_signature(needs_signature)
{
    std::string f = filename;
    std::replace(f.begin(), f.end(), '.', '-');

    // 1. Determine Directory
    auto it = g_type_map.find(m_type);
    path target_dir = (it != g_type_map.end()) ? it->second.dir : g_autogen_dir;
    m_filename = (target_dir / (f + ".htm")).string();

    // 2. Sanitize Filename
    auto sanitize = [this](const std::string& chars, const std::string& replacements) {
        for (size_t i = 0; i < chars.size(); ++i) {
            std::replace(m_filename.begin(), m_filename.end(), chars[i], replacements[i]);
        }
    };
    sanitize("(),<>{}", "__-____");

    m_filename.erase(std::remove_if(m_filename.begin(), m_filename.end(), [](unsigned char c) {
        return std::isspace(c) || c == '\n' || c == '\r';
    }), m_filename.end());
}

auto Topic::get_topic_name() -> const std::string &
{
    return m_topic_name;
}

auto Topic::get_keyword() -> const std::string &
{
    return m_keyword;
}

auto Topic::get_filename() -> const std::string &
{
    return m_filename;
}

auto Topic::get_topic_type() -> TopicType
{
    return m_type;
}

// Helper functions for legibility
XMLElement* add_element(XMLNode* parent, const char* name, const char* text = nullptr) {
    auto el = parent->GetDocument()->NewElement(name);
    if (text) el->SetText(text);
    parent->InsertEndChild(el);
    return el;
}

void add_styled_section(XMLNode* parent, const std::string& title) {
    auto h2 = add_element(parent, "h2");
    h2->SetAttribute("class", "p_Heading2");
    auto span = add_element(h2, "span", title.c_str());
    span->SetAttribute("class", "f_Heading2");
    
    auto p = add_element(parent, "p", g_placeholder.c_str());
    p->SetAttribute("class", "p_Normal");
}

// Main generation logic
auto Topic::create_topic(std::string prev_topic, std::string next_topic) -> bool
{
    if (exists(path(m_filename))) {
        std::cout << "File '" << m_filename << "' exists, skipping...\n";
        return true;
    }

    XMLDocument doc;
    doc.InsertFirstChild(doc.NewDeclaration());

    auto root = add_element(&doc, "html");
    root->SetAttribute("xmlns:MadCap", "http://www.madcapsoftware.com/Schemas/MadCap.xsd");

    // Head Section
    auto head = add_element(root, "head");
    std::string display_name = m_topic_name + (g_type_map.count(m_type) ? g_type_map.at(m_type).suffix : "");
    add_element(head, "title", display_name.c_str());

    auto meta_k = add_element(head, "meta");
    meta_k->SetAttribute("name", "keywords");
    meta_k->SetAttribute("content", m_keyword.c_str());

    auto link = add_element(head, "link");
    link->SetAttribute("rel", "stylesheet");
    link->SetAttribute("href", "../../Resources/Stylesheets/default.css");

    auto style = add_element(head, "style", "body { margin: 0px; background: #FFFFFF; }\n");
    style->SetAttribute("type", "text/css");

    // Body Section
    auto body = add_element(root, "body");
    
    // Header Table
    auto table = add_element(body, "table");
    table->SetAttribute("style", ("width: 100%; border: none; padding: 5px; background: " + g_header_color).c_str());
    auto tr = add_element(table, "tr");
    tr->SetAttribute("style", "vertical-align: middle;");

    auto td_left = add_element(tr, "td");
    td_left->SetAttribute("style", "text-align: left;");
    
    // Heading with MadCap Keywords
    auto h1 = add_element(td_left, "h1");
    h1->SetAttribute("class", "p_Heading1");
    auto kw1 = add_element(h1, "MadCap:keyword");
    kw1->SetAttribute("term", m_keyword.c_str());
    auto span1 = add_element(h1, "span", display_name.c_str());
    span1->SetAttribute("class", "f_Heading1");

    auto td_right = add_element(tr, "td");
    td_right->SetAttribute("style", "text-align: right;");
    auto a_top = add_element(td_right, "a", "Top");
    a_top->SetAttribute("href", "../../welcome.htm");

    if (m_type == TopicType::domain_object || m_type == TopicType::function) {
        add_element(td_right, "a", "Previous")->SetAttribute("href", prev_topic.c_str());
        add_element(td_right, "a", "Next")->SetAttribute("href", next_topic.c_str());
    }

    // Content Generation
    add_styled_section(body, "Description");

    if (m_type == TopicType::domain_object) {
        add_element(body, "b", "Inheritance Hierarchy:");
        add_element(body, "p", g_placeholder.c_str())->SetAttribute("class", "p_Normal");
        add_styled_section(body, "Available In Editions:");
        
        // Collapsible sections
        for (const auto& section : {"Constructors", "Properties", "Methods"}) {
            auto det = add_element(body, "details");
            auto sum = add_element(det, "summary");
            auto h = add_element(sum, "h2");
            h->SetAttribute("class", "p_Heading2");
            add_element(h, "span", section)->SetAttribute("class", "f_Heading2");
            add_element(det, "p", g_placeholder.c_str())->SetAttribute("class", "p_Normal");
        }
    } 
    else if (m_type == TopicType::property) {
        add_styled_section(body, "Attributes");
        add_styled_section(body, "Syntax");
    }
    else if (m_type == TopicType::method || m_type == TopicType::function || m_type == TopicType::constructor) {
        if (m_needs_signature) {
            add_styled_section(body, "Signature");
            add_styled_section(body, "Arguments");
            if (m_type != TopicType::constructor)
                add_styled_section(body, "Return Value");
            add_styled_section(body, "Syntax");
        } else {
            add_styled_section(body, "Overload List");
        }
    }

    add_styled_section(body, "See also");

    // File I/O
    return doc.SaveFile(m_filename.c_str()) == XML_SUCCESS;
}