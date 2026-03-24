#pragma once

#include "topic.hpp"
#include "common_includes.hpp"

class TableOfContent
{
public:
    TableOfContent(const std::vector<Topic> &topics, const std::string &filename);
    auto create_toc() -> bool;

private:
    std::vector<Topic> m_topics;
    std::string m_filename;
};