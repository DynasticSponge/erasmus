//
// data_match.hpp
// ~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATA_MATCH_HPP
#define DATA_MATCH_HPP

#include <string>
#include <vector>

#include "erasmus_namespace.hpp"

class erasmus::dataMatch
{
public:
    dataMatch(const std::string&, size_t, size_t);
    bool addChar(char);
    size_t getDistance();
    size_t getLength();
    ~dataMatch();
protected:
private:
    bool active;
    size_t matchEnd;
    size_t matchStart;
    size_t previousStart;
    std::string matchString;
    const std::string& source;
};

#endif