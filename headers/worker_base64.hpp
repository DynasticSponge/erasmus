//
// worker_base64.hpp
// ~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WORKER_BASE64_HPP
#define WORKER_BASE64_HPP

#include <string>

#include "erasmus_namespace.hpp"

class erasmus::workerBase64
{
public:
    workerBase64();
    bool base64Decode(const std::string&, std::string&);
    bool base64Encode(const std::string&, std::string&);
    bool mimeEncode(const std::string&, std::string&);
    ~workerBase64();
protected:
private:
    ///////////////////////////////////////////////////////////////////////////////
    // Private Functions
    ///////////////////////////////////////////////////////////////////////////////
    void getGroup(const std::string&, std::string&, size_t, size_t);
    size_t getNumGroups(const std::string&, size_t);
    void removeWhiteSpace(const std::string&, std::string&);
    ///////////////////////////////////////////////////////////////////////////////
    // Private Properties
    ///////////////////////////////////////////////////////////////////////////////
    std::string alphabet;
};

#endif