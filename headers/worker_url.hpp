//
// worker_url.hpp
// ~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WORKER_URL_HPP
#define WORKER_URL_HPP

#include <string>

#include "erasmus_namespace.hpp"

class erasmus::workerURL
{
public:
    explicit workerURL();
    explicit workerURL(erasmus::director*);
    bool urlDecode(const std::string&, std::string&);
    bool urlEncode(const std::string&, std::string&);
    ~workerURL();
protected:
private:
    ///////////////////////////////////////////////////////////////////////////////
    // Private Functions
    ///////////////////////////////////////////////////////////////////////////////
    bool charDecode(const std::string&, char&);
    bool charEncode(char, std::string&);
    bool initNumericWorker();
    ///////////////////////////////////////////////////////////////////////////////
    // Private Properties
    ///////////////////////////////////////////////////////////////////////////////
    erasmus::director *director;
    erasmus::workerNumeric *numericWorker;
};

#endif