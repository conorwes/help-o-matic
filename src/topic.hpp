#pragma once

#include "common_includes.hpp"

class Topic
{
public:
    Topic(std::string topic_name, std::string keyword, std::string filename, TopicType type, bool needs_signature = false);
    auto get_topic_name() const -> const std::string &;
    auto get_keyword() const -> const std::string &;
    auto get_filename() const -> const std::string &;
    auto get_topic_type() const noexcept -> TopicType;
    auto create_topic(const std::string &prev_topic = "", const std::string &next_topic = "") -> bool;

private:
    std::string m_topic_name;
    std::string m_keyword;
    std::string m_filename;
    TopicType m_type;
    bool m_needs_signature;
};