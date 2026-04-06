#pragma once

#include "common_includes.hpp"

enum class TopicType
{
    domain_object,
    function,
    property,
    method,
    constructor
};

class Topic
{
public:
    Topic(std::string topic_name, std::string keyword, std::string filename, TopicType type, bool needs_signature=false);
    auto get_topic_name() -> const std::string &;
    auto get_keyword() -> const std::string &;
    auto get_filename() -> const std::string &;
    auto get_topic_type() -> TopicType;
    auto create_topic(std::string prev_topic="", std::string next_topic="") -> bool;

private:
    std::string m_topic_name;
    std::string m_keyword;
    std::string m_filename;
    TopicType m_type;
    bool m_needs_signature;
};