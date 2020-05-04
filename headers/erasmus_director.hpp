//
// erasmus_director.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ERASMUS_DIRECTOR_HPP
#define ERASMUS_DIRECTOR_HPP

#include <string>

#include "erasmus_namespace.hpp"

class erasmus::director
{
public:
    director();
    bool base64Decode(const std::string&, std::string&);
    bool base64Encode(const std::string&, std::string&);
    bool charToHex(char, std::string&);
    bool escapeReplace(const std::string&, std::string&);
    bool isHex(char);
    bool mimeEncode(const std::string&, std::string&);
    bool quotedExtract(const std::string&, std::string&, bool);
    bool urlDecode(const std::string&, std::string&);
    bool urlEncode(const std::string&, std::string&);
    bool tokenReplace(std::string&, const std::string&, const std::string&);
    bool toLower(const std::string&, std::string&);
    bool toUpper(const std::string&, std::string&);
    bool uintToHex(size_t, std::string&);
    ~director();
protected:
private:
    workerBase64 *base64Worker;
    workerCase *caseWorker;
    workerExtract *extractWorker;
    workerNumeric *numericWorker;
    workerReplace *replaceWorker;
    workerURL *urlWorker;
};

#endif