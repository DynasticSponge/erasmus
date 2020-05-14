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
#include <string_view>
#include <vector>

#include "erasmus_namespace.hpp"

class erasmus::dataMatch
{
public:
    dataMatch(size_t, size_t, size_t);
    size_t getDistance();
    size_t getLength();
    size_t getPrevious();
    size_t getStart();
    void postBack(size_t);
    void trimBack(size_t);
    void trimFront(size_t);
    ~dataMatch();
protected:
private:
    size_t matchStart;
    size_t matchLength;
    size_t firstStart;
};

#endif