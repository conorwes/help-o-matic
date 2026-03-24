
#include "toc.hpp"

using namespace tinyxml2;
using namespace std::filesystem;

TableOfContent::TableOfContent(const std::vector<Topic> &topics, const std::string &filename)
{
    m_topics = topics;
    m_filename = (g_toc_dir / (filename + ".fltoc")).string();
}

auto TableOfContent::create_toc() -> bool
{
    XMLDocument doc;
    XMLDeclaration *decl = doc.NewDeclaration();
    doc.InsertFirstChild(decl);

    XMLNode *root = doc.NewElement("CatapultToc");
    auto root_elem = root->ToElement();
    root_elem->SetAttribute("Version", "1");
    doc.InsertEndChild(root);

    for (auto t : m_topics)
    {
        auto e = doc.NewElement("TocEntry");
        e->SetAttribute("Title", t.get_topic_name().c_str());
        e->SetAttribute("HtmlHelpIconIndex", "11");
        e->SetAttribute("Link", t.get_filename().c_str());
        root->InsertEndChild(e);
    }

    auto result = doc.SaveFile(m_filename.c_str());
    if (result != XML_SUCCESS)
        return false;

    return true;
}