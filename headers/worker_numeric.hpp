//
// worker_numeric.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WORKER_NUMERIC_HPP
#define WORKER_NUMERIC_HPP

#include <string>

#include "erasmus_namespace.hpp"

class erasmus::workerNumeric
{
public:
    workerNumeric();
    bool charToHex(char, std::string&);
    bool isHex(char);
    bool uintToHex(size_t, std::string&);
    ~workerNumeric();
protected:
private:
};

#endif