//
// worker_case.cpp
// ~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <exception>
#include <string>

#include "../headers/erasmus_namespace.hpp"
#include "../headers/worker_case.hpp"

using namespace erasmus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global variable definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus::workerCase member definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////

workerCase::workerCase()
{

}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerCase::setCase
///////////////////////////////////////////////////////////////////////////////

bool workerCase::setCase(const std::string& original, std::string& revised, bool upper)
{
    bool returnValue{true};
    std::string bldStr{original};
    
    try
    {
        auto lFunc = [](unsigned char c){return(std::tolower(c));};
        auto uFunc = [](unsigned char c){return(std::toupper(c));};
        if(upper)
        {
            std::transform(bldStr.begin(), bldStr.end(), bldStr.begin(), uFunc);
        }
        else
        {
            std::transform(bldStr.begin(), bldStr.end(), bldStr.begin(), lFunc);
        }       
    }
    catch(const std::exception& e)
    {
        returnValue = false;
    }
    
    if(returnValue)
    {
        revised = std::move(bldStr);
    }
    
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerCase::toLower
///////////////////////////////////////////////////////////////////////////////

bool workerCase::toLower(const std::string& original, std::string& revised)
{
    bool returnValue{this->setCase(original, revised, false)};
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerCase::toUpper
///////////////////////////////////////////////////////////////////////////////

bool workerCase::toUpper(const std::string& original, std::string& revised)
{
    bool returnValue{this->setCase(original, revised, true)};
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerCase::~workerCase()
{

}