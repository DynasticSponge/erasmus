//
// data_node.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATA_NODE_HPP
#define DATA_NODE_HPP

#include <vector>
#include "erasmus_namespace.hpp"

class erasmus::dataNode
{
public:
    dataNode();
    bool operator<(const erasmus::dataNode&) const;
    ~dataNode();
    unsigned short code;
    unsigned short hasKids;
    size_t weight;
    std::vector<erasmus::dataNode> kids;
protected:
private:
};

#endif