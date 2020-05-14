//
// worker_case.hpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WORKER_CASE_HPP
#define WORKER_CASE_HPP

#include <string>

#include "erasmus_namespace.hpp"

class erasmus::workerCase
{
public:
    explicit workerCase();
    bool toLower(const std::string&, std::string&);
    bool toUpper(const std::string&, std::string&);
    ~workerCase();
protected:
private:
    bool setCase(const std::string&, std::string&, bool);
};

#endif