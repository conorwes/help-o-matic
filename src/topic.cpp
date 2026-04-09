#include "topic.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

Topic::Topic(std::string topic_name, std::string keyword, std::string filename, TopicType type, bool needs_signature)
    : m_topic_name(std::move(topic_name)), m_keyword(std::move(keyword)), m_type(type), m_needs_signature(needs_signature)
{
    std::string f = filename;
    std::replace(f.begin(), f.end(), '.', '-');

    // Determine the directory
    auto it = g_type_map.find(m_type);
    path target_dir = (it != g_type_map.end()) ? it->second.dir : g_autogen_dir;
    m_filename = (target_dir / (f + ".htm")).string();

    // Sanitize the filename
    sanitize_filename(m_filename, g_bad_chars, g_good_chars);
    m_filename.erase(std::remove_if(m_filename.begin(), m_filename.end(), [](unsigned char c)
                                    { return std::isspace(c) || c == '\n' || c == '\r'; }),
                     m_filename.end());
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
XMLElement *add_element(XMLNode *parent, const char *name, const char *text = nullptr)
{
    auto el = parent->GetDocument()->NewElement(name);
    if (text)
        el->SetText(text);
    parent->InsertEndChild(el);
    return el;
}

void add_styled_section(XMLNode *parent, const std::string &title)
{
    auto h2 = add_element(parent, "h2");
    h2->SetAttribute("class", "p_Heading2");
    auto span = add_element(h2, "span", title.c_str());
    span->SetAttribute("class", "f_Heading2");

    auto p = add_element(parent, "p", g_placeholder.c_str());
    p->SetAttribute("class", "p_Normal");
}

void add_collapsable_section(XMLNode *parent, const std::string &text, const std::string &toggle_id)
{
    // 1. The toggle button and label
    auto toggle = add_element(parent, "p");
    toggle->SetAttribute("class", "p_Normal");
    toggle->SetAttribute("style", "text-indent: -0.1875in;margin: 0 0 0.0208in 0.1875in;");

    auto img = add_element(toggle, "img");
    img->SetAttribute("id", (toggle_id + "_ICON").c_str());
    img->SetAttribute("class", "dropdown-toggle-icon");
    img->SetAttribute("style", "margin: 0; width: 0.0938in;height: 0.0938in;border:none;");
    img->SetAttribute("src", "../../Resources/Images/hmtoggle_plus1.gif");

    auto a = add_element(img, "a");
    a->SetAttribute("class", "dropdown-toggle");
    a->SetAttribute("style", "font-style: normal;font-weight: normal;color: #000000;background-color: transparent;text-decoration: none;"); // TODO - do we need any of the styling at this level?
    a->SetAttribute("href", ("javascript:HMToggle('toggle,'" + toggle_id + "','" + toggle_id + "_ICON')").c_str());

    auto span = add_element(a, "span", text.c_str());
    span->SetAttribute("class", "f_Heading2");

    // 2. The toggle body
    auto div = add_element(parent, "div");
    div->SetAttribute("id", toggle_id.c_str());
    div->SetAttribute("class", "dropdown-toggle-body");
    div->SetAttribute("style", "text-align: left;padding: 0 0 0 0;margin: 0 0 0.0208in 0.1875in;");

    auto p = add_element(div, "p");
    p->SetAttribute("class", "p_Normal");
    p->SetText(g_placeholder.c_str());
}

// Main generation logic
auto Topic::create_topic(std::string prev_topic, std::string next_topic) -> bool
{
    if (exists(path(m_filename)))
    {
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

    auto meta_h = add_element(head, "meta");
    meta_h->SetAttribute("http-equiv", "X-UA-Compatible");
    meta_h->SetAttribute("content", "IE=edge");

    auto link = add_element(head, "link");
    link->SetAttribute("type", "text/css");
    link->SetAttribute("href", "../../Resources/Stylesheets/default.css");
    link->SetAttribute("rel", "stylesheet");

    auto style = add_element(head, "style", "body { margin: 0px; background: #FFFFFF; }\n");
    style->SetAttribute("type", "text/css");

    // Scripts Section
    // TODO - Figure out why these scripts require that they be at the same level as the page itself - this is a problem with the script, not the generator
    auto script_1 = doc.NewElement("script");
    script_1->SetAttribute("type", "text/javascript");
    script_1->SetAttribute("src", "Resources/jquery.js");
    script_1->InsertEndChild(doc.NewText("")); // Force full closing tag
    head->InsertEndChild(script_1);

    auto script_2 = doc.NewElement("script");
    script_2->SetAttribute("type", "text/javascript");
    script_2->SetAttribute("src", "Resources/helpman_settings.js");
    script_2->InsertEndChild(doc.NewText("")); // Force full closing tag
    head->InsertEndChild(script_2);

    auto script_3 = doc.NewElement("script");
    script_3->SetAttribute("type", "text/javascript");
    script_3->SetAttribute("src", "Resources/helpman_topicinit.js");
    script_3->InsertEndChild(doc.NewText("")); // Force full closing tag
    head->InsertEndChild(script_3);

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

    if (m_type == TopicType::domain_object || m_type == TopicType::function || m_type == TopicType::available_dos || m_type == TopicType::available_funcs)
    {
        add_element(td_right, "a", "Previous")->SetAttribute("href", prev_topic.c_str());
        add_element(td_right, "a", "Next")->SetAttribute("href", next_topic.c_str());
    }

    auto placeholder = add_element(body, "table");
    placeholder->SetAttribute("style", "width: 100%;border: none;border-spacing: 0px;");

    auto tr_placeholder = add_element(placeholder, "tr");
    tr_placeholder->SetAttribute("style", "vertical-align: top;");

    auto td_placeholder = add_element(tr_placeholder, "td");
    td_placeholder->SetAttribute("style", "text-align: left;padding: 5px;");

    auto id = "toggle0186a";
    int id_suffix = 1;
    auto num_toggles = 0;

    // If this is an "Available Objects" or "Available Functions" page, just put in the placeholder text
    if (m_type == TopicType::available_dos || m_type == TopicType::available_funcs)
    {
        add_element(td_placeholder, "p", g_placeholder.c_str())->SetAttribute("class", "p_Normal");
    }
    else
    {
        // Content Generation
        add_styled_section(td_placeholder, "Description");

        if (m_type == TopicType::domain_object)
        {
            add_element(td_placeholder, "b", "Inheritance Hierarchy:");
            add_element(td_placeholder, "p", g_placeholder.c_str())->SetAttribute("class", "p_Normal");
            add_styled_section(td_placeholder, "Available In Editions:");

            // Collapsible sections
            for (const auto &section : {"Constructors", "Properties", "Methods"})
            {
                add_collapsable_section(td_placeholder, section, id + std::to_string(id_suffix));
                id_suffix++;
                num_toggles++;
            }
        }
        else if (m_type == TopicType::property)
        {
            add_styled_section(td_placeholder, "Attributes");
            add_styled_section(td_placeholder, "Syntax");
        }
        else if (m_type == TopicType::method || m_type == TopicType::function || m_type == TopicType::constructor)
        {
            if (m_needs_signature)
            {
                add_styled_section(td_placeholder, "Signature");
                add_styled_section(td_placeholder, "Arguments");
                if (m_type != TopicType::constructor)
                    add_styled_section(td_placeholder, "Return Value");
                add_styled_section(td_placeholder, "Syntax");
            }
            else
            {
                add_styled_section(td_placeholder, "Overload List");
            }
        }

        add_styled_section(td_placeholder, "See also");
    }

    std::string script_text = "\nif (window.topicInitScriptAvailable) {"
                              "\nfunction HMInitToggleStates() {";

    for (auto i = 0; i < num_toggles; i++)
    {
        auto toggle_id = std::string(id) + std::to_string(i + 1);
        script_text += std::string("\nHMInitToggle('" + toggle_id + "_ICON','hm.type','dropdown','hm.state','0','hm.src0','hmtoggle_plus0.gif','hm.src1','hmtoggle_plus1.gif','onclick','HMToggle(\\'toggle\\',\\'" + toggle_id + "\\',\\'" + toggle_id + "_ICON\\')');");
        script_text += std::string("\nHMInitToggle('" + toggle_id + "','hm.type','dropdown','hm.state','0');");
    }

    script_text += "\n}"
                   "\nvar checkTimer=setTimeout(\"HMInitToggleStates()\",0);"
                   "\n}\n";

    auto script_final = add_element(body, "script");
    script_final->SetAttribute("type", "text/javascript");

    // 1. Create the opening '/*' as a plain text node
    script_final->InsertEndChild(doc.NewText("/*"));

    // 2. Create the CDATA node.
    // To get '/*<![CDATA[*/', the CDATA content must START with '*/'
    // To get '/*]]>*/', the CDATA content must END with '/*'
    std::string cdata_content = "*/\n" + script_text + "\n/*";
    auto cdata_node = doc.NewText(cdata_content.c_str());
    cdata_node->SetCData(true);
    script_final->InsertEndChild(cdata_node);

    // 3. Create the closing '*/' as a plain text node
    script_final->InsertEndChild(doc.NewText("*/"));

    // File I/O
    XMLPrinter printer(0, false);
    doc.Print(&printer);
    std::ofstream of(m_filename.c_str());
    of << printer.CStr();
    of.close();

    return true;
}