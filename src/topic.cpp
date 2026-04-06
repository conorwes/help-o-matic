
#include <cctype>

#include "topic.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

Topic::Topic(std::string topic_name, std::string keyword, std::string filename, TopicType type, bool needs_signature)
{
    m_topic_name = topic_name;
    m_keyword = keyword;
    m_type = type;
    m_needs_signature = needs_signature;
    auto f = filename;
    std::replace(f.begin(), f.end(), '.', '-');

    switch (m_type)
    {
    case TopicType::domain_object:
        m_filename = (g_do_dir / (f + ".htm")).string();
        break;

    case TopicType::constructor:
        m_filename = (g_cons_dir / (f + ".htm")).string();
        break;

    case TopicType::property:
        m_filename = (g_props_dir / (f + ".htm")).string();
        break;

    case TopicType::method:
        m_filename = (g_meths_dir / (f + ".htm")).string();
        break;

    case TopicType::function:
        m_filename = (g_funcs_dir / (f + ".htm")).string();
        break;

    default:
        topic_name = "Error Type";
        m_filename = (g_autogen_dir / (f + ".htm")).string();
        break;
    }

    auto orig = std::string("(),<>{}");
    auto repl = std::string("__-____");

    size_t index = 0;
    for (const auto &c : orig)
    {
        if (m_filename.find(c) != std::string::npos)
        {
            std::replace(m_filename.begin(), m_filename.end(), c, repl[index]);
        }

        index++;
    }

    m_filename.erase(std::remove(m_filename.begin(), m_filename.end(), '\n'), m_filename.end());
    m_filename.erase(std::remove(m_filename.begin(), m_filename.end(), '\r'), m_filename.end());
    m_filename.erase(std::remove_if(m_filename.begin(), m_filename.end(), ::isspace), m_filename.end());
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

auto Topic::create_topic(std::string prev_topic, std::string next_topic) -> bool
{
    // before we do anything, make sure the file doesn't already exist - we don't want to blow away any manual content
    if (std::filesystem::exists(m_filename))
    {
        std::cout << "File '" + m_filename + "' already exists, skipping...";
        return 1;
    }

    XMLDocument doc;
    XMLDeclaration *decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    XMLNode *root = doc.NewElement("html");
    auto root_elem = root->ToElement();
    root_elem->SetAttribute("xmlns:MadCap", "http://www.madcapsoftware.com/Schemas/MadCap.xsd");
    doc.InsertEndChild(root);

    auto head = doc.NewElement("head");
    root->InsertFirstChild(head);

    auto title = doc.NewElement("title");
    auto topic_name = m_topic_name;

    switch (m_type)
    {
    case TopicType::domain_object:
        topic_name += " Object";
        break;

    case TopicType::constructor:
        topic_name += " Constructor";
        break;

    case TopicType::property:
        topic_name += " Property";
        break;

    case TopicType::method:
        topic_name += " Method";
        break;

    case TopicType::function:
        topic_name += " Function";
        break;

    default:
        return 1;
    }

    title->SetText(topic_name.c_str());
    head->InsertFirstChild(title);

    auto keywords = doc.NewElement("meta");
    keywords->SetAttribute("name", "keywords");
    keywords->SetAttribute("content", m_keyword.c_str());
    head->InsertEndChild(keywords);

    auto http_equiv = doc.NewElement("meta");
    http_equiv->SetAttribute("http-equiv", "X-UA-Compatible");
    http_equiv->SetAttribute("content", "IE=edge");
    head->InsertEndChild(http_equiv);

    auto link = doc.NewElement("link");
    link->SetAttribute("type", "text/css");
    link->SetAttribute("href", "../../Resources/Stylesheets/default.css");
    link->SetAttribute("rel", "stylesheet");
    head->InsertEndChild(link);

    auto style = doc.NewElement("style");
    style->SetAttribute("type", "text/css");
    style->SetText(std::string("body\n{\n	margin: 0px;\n	background: #FFFFFF;\n}\n\n").c_str());
    head->InsertEndChild(style);

    /* temporarily comment this out
    auto script_1 = doc.NewElement("script");
    script_1->SetAttribute("type", "text/javascript");
    script_1->SetAttribute("src", "../../Resources/jquery.js");
    head->InsertEndChild(script_1);

    auto script_2 = doc.NewElement("script");
    script_2->SetAttribute("type", "text/javascript");
    script_2->SetAttribute("src", "../../Resources/helpman_settings.js");
    head->InsertEndChild(script_2);

    auto script_3 = doc.NewElement("script");
    script_3->SetAttribute("type", "text/javascript");
    script_3->SetAttribute("src", "../../Resources/helpman_topicinit.js");
    head->InsertEndChild(script_3);*/

    auto body = doc.NewElement("body");
    root->InsertEndChild(body);

    // Header
    auto header_table = doc.NewElement("table");
    auto style_text = "width: 100%;border: none;border-spacing: 0px;padding: 5px;background: " + g_header_color + ";";
    header_table->SetAttribute("style", style_text.c_str());

    auto tr = doc.NewElement("tr");
    tr->SetAttribute("style", "vertical-align: middle;");

    auto td1 = doc.NewElement("td");
    td1->SetAttribute("style", "text-align: left;");

    auto td2 = doc.NewElement("td");
    td2->SetAttribute("style", "text-align: right;");
    auto a1 = doc.NewElement("a");
    a1->SetAttribute("href", "../../welcome.htm");
    a1->SetText("Top");
    td2->InsertFirstChild(a1);

    if (m_type == TopicType::domain_object || m_type == TopicType::function)
    {
        auto a2 = doc.NewElement("a");
        a2->SetAttribute("href", prev_topic.c_str());
        a2->SetText("Previous");
        auto a3 = doc.NewElement("a");
        a3->SetAttribute("href", next_topic.c_str());
        a3->SetText("Next");
        td2->InsertEndChild(a2);
        td2->InsertEndChild(a3);
    }

    auto h1 = doc.NewElement("h1");
    h1->SetAttribute("class", "p_Heading1");

    auto h1_keyword = doc.NewElement("MadCap:keyword");
    h1_keyword->SetAttribute("term", m_keyword.c_str());

    auto h1_span = doc.NewElement("span");
    h1_span->SetAttribute("class", "f_Heading1");
    h1_span->SetText(topic_name.c_str());

    auto h1_span_keyword = doc.NewElement("MadCap:keyword");
    h1_span_keyword->SetAttribute("term", m_keyword.c_str());

    h1_span->InsertFirstChild(h1_span_keyword);
    h1->InsertFirstChild(h1_keyword);
    h1->InsertEndChild(h1_span);
    td1->InsertFirstChild(h1);
    tr->InsertFirstChild(td1);
    tr->InsertEndChild(td2);
    header_table->InsertFirstChild(tr);
    body->InsertFirstChild(header_table);

    /*auto h1 = doc.NewElement("h1");
    h1->SetText(topic_name.c_str());
    body->InsertFirstChild(h1);*/

    // Description
    auto desc = doc.NewElement("h2");
    desc->SetText("Description");
    body->InsertEndChild(desc);

    auto desc_text = doc.NewElement("p");
    desc_text->SetText(g_placeholder.c_str());
    body->InsertEndChild(desc_text);

    if (m_type == TopicType::domain_object)
    {
        // Inheritance Hierarchy
        auto ih = doc.NewElement("b");
        ih->SetText("Ineritance Hierarchy:");
        body->InsertEndChild(ih);

        auto ih_text = doc.NewElement("p");
        ih_text->SetText(g_placeholder.c_str());
        body->InsertEndChild(ih_text);

        auto po = doc.NewElement("b");
        po->SetText("Parent Object:");
        body->InsertEndChild(po);

        auto po_text = doc.NewElement("p");
        po_text->SetText(g_placeholder.c_str());
        body->InsertEndChild(po_text);

        // Editions
        auto edits = doc.NewElement("h2");
        edits->SetText("Available In Editions:");
        body->InsertEndChild(edits);
        auto editions = doc.NewElement("p");
        editions->SetText(g_placeholder.c_str());
        body->InsertEndChild(editions);
    }

    if (m_type == TopicType::domain_object)
    {
        // Constructors
        auto det = doc.NewElement("details");
        auto sum = doc.NewElement("summary");
        auto sb = doc.NewElement("h2");
        sb->SetText("Constructors");
        sum->InsertFirstChild(sb);
        auto cont = doc.NewElement("p");
        cont->SetText(g_placeholder.c_str());
        det->InsertFirstChild(cont);
        det->InsertFirstChild(sum);
        body->InsertEndChild(det);

        // Properties
        auto det2 = doc.NewElement("details");
        auto sum2 = doc.NewElement("summary");
        auto sb2 = doc.NewElement("h2");
        sb2->SetText("Properties");
        sum2->InsertFirstChild(sb2);
        auto cont2 = doc.NewElement("p");
        cont2->SetText(g_placeholder.c_str());
        det2->InsertFirstChild(cont2);
        det2->InsertFirstChild(sum2);
        body->InsertEndChild(det2);

        // Methods
        auto det3 = doc.NewElement("details");
        auto sum3 = doc.NewElement("summary");
        auto sb3 = doc.NewElement("h2");
        sb3->SetText("Methods");
        sum3->InsertFirstChild(sb3);
        auto cont3 = doc.NewElement("p");
        cont3->SetText(g_placeholder.c_str());
        det3->InsertFirstChild(cont3);
        det3->InsertFirstChild(sum3);
        body->InsertEndChild(det3);
    }
    else if (m_type == TopicType::property)
    {
        // Attributes
        auto at_h2 = doc.NewElement("h2");
        at_h2->SetText("Attributes");
        body->InsertEndChild(at_h2);
        auto attributes = doc.NewElement("p");
        attributes->SetText(g_placeholder.c_str());
        body->InsertEndChild(attributes);

        // Syntax
        auto sy_h2 = doc.NewElement("h2");
        sy_h2->SetText("Syntax");
        body->InsertEndChild(sy_h2);
        auto syntax = doc.NewElement("p");
        syntax->SetText(g_placeholder.c_str());
        body->InsertEndChild(syntax);
    }
    else if (m_type == TopicType::method)
    {
        if (m_needs_signature)
        {
            // Method Signature
            auto ms_h2 = doc.NewElement("h2");
            ms_h2->SetText("Method Signature");
            body->InsertEndChild(ms_h2);
            auto ms = doc.NewElement("p");
            ms->SetText(m_topic_name.c_str());
            body->InsertEndChild(ms);

            // Arguments
            auto ar_h2 = doc.NewElement("h2");
            ar_h2->SetText("Arguments");
            body->InsertEndChild(ar_h2);
            auto arguments = doc.NewElement("p");
            arguments->SetText(g_placeholder.c_str());
            body->InsertEndChild(arguments);

            // Return Value
            auto rv_h2 = doc.NewElement("h2");
            rv_h2->SetText("Return Value");
            body->InsertEndChild(rv_h2);
            auto value = doc.NewElement("p");
            value->SetText(g_placeholder.c_str());
            body->InsertEndChild(value);

            // Syntax
            auto s_h2 = doc.NewElement("h2");
            s_h2->SetText("Syntax");
            body->InsertEndChild(s_h2);
            auto syntax = doc.NewElement("p");
            syntax->SetText(g_placeholder.c_str());
            body->InsertEndChild(syntax);
        }
        else
        {
            // Overload List
            auto ol_h2 = doc.NewElement("h2");
            ol_h2->SetText("Overload List");
            body->InsertEndChild(ol_h2);
            auto olist = doc.NewElement("p");
            olist->SetText(g_placeholder.c_str());
            body->InsertEndChild(olist);
        }
    }
    else if (m_type == TopicType::constructor)
    {
        if (m_needs_signature)
        {
            // Constructor Signature
            auto cs_h2 = doc.NewElement("h2");
            cs_h2->SetText("Constructor Signature");
            body->InsertEndChild(cs_h2);
            auto cs = doc.NewElement("p");
            cs->SetText(m_topic_name.c_str());
            body->InsertEndChild(cs);
        }

        // Arguments
        auto ar_h2 = doc.NewElement("h2");
        ar_h2->SetText("Arguments");
        body->InsertEndChild(ar_h2);
        auto arguments = doc.NewElement("p");
        arguments->SetText(g_placeholder.c_str());
        body->InsertEndChild(arguments);

        // Syntax
        auto sy_h2 = doc.NewElement("h2");
        sy_h2->SetText("Syntax");
        body->InsertEndChild(sy_h2);
        auto syntax = doc.NewElement("p");
        syntax->SetText(g_placeholder.c_str());
        body->InsertEndChild(syntax);
    }
    else if (m_type == TopicType::function)
    {
        if (m_needs_signature)
        {
            // Function Signature
            auto fs_h2 = doc.NewElement("h2");
            fs_h2->SetText("Function Signature");
            body->InsertEndChild(fs_h2);
            auto fs = doc.NewElement("p");
            fs->SetText(m_topic_name.c_str());
            body->InsertEndChild(fs);

            // Arguments
            auto ar_h2 = doc.NewElement("h2");
            ar_h2->SetText("Arguments");
            body->InsertEndChild(ar_h2);
            auto arguments = doc.NewElement("p");
            arguments->SetText(g_placeholder.c_str());
            body->InsertEndChild(arguments);

            // Return Value
            auto rv_h2 = doc.NewElement("h2");
            rv_h2->SetText("Return Value");
            body->InsertEndChild(rv_h2);
            auto value = doc.NewElement("p");
            value->SetText(g_placeholder.c_str());
            body->InsertEndChild(value);

            // Syntax
            auto s_h2 = doc.NewElement("h2");
            s_h2->SetText("Syntax");
            body->InsertEndChild(s_h2);
            auto syntax = doc.NewElement("p");
            syntax->SetText(g_placeholder.c_str());
            body->InsertEndChild(syntax);
        }
        else
        {
            // Overload List
            auto ol_h2 = doc.NewElement("h2");
            ol_h2->SetText("Overload List");
            body->InsertEndChild(ol_h2);
            auto olist = doc.NewElement("p");
            olist->SetText(g_placeholder.c_str());
            body->InsertEndChild(olist);
        }
    }

    // See Also
    auto seealso = doc.NewElement("h2");
    seealso->SetText("See also");
    body->InsertEndChild(seealso);
    auto sap = doc.NewElement("p");
    sap->SetText(g_placeholder.c_str());
    body->InsertEndChild(sap);

    FILE *fp = nullptr;
    errno_t err = fopen_s(&fp, m_filename.c_str(), "w");
    if (err != 0)
        return false;

    tinyxml2::XMLPrinter printer(fp);
    doc.Print(&printer);
    fclose(fp);

    return true;
}