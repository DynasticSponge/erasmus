//
// worker_replace.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WORKER_REPLACE_HPP
#define WORKER_REPLACE_HPP

#include <string>

#include "erasmus_namespace.hpp"

class erasmus::workerReplace
{
public:
    workerReplace();
    bool escapeReplace(const std::string&, std::string&);
    bool tokenReplace(std::string&, const std::string&, const std::string&);
    ~workerReplace();
protected:
private:
};

#endif