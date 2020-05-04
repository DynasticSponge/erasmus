//
// data_window.hpp
// ~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DATA_WINDOW_HPP
#define DATA_WINDOW_HPP

#include <string>
#include <vector>

#include "erasmus_namespace.hpp"

class erasmus::dataWindow
{
public:
    dataWindow(const std::string&, size_t);
    void addChar();
    std::vector<size_t> getNewMatches();
    ~dataWindow();
protected:
private:
    size_t end;
    size_t maxSize;
    size_t start;
    const std::string& source;
};

#endif