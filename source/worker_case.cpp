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
// erasmus::workerCase::toLower
///////////////////////////////////////////////////////////////////////////////

bool workerCase::toLower(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    std::string bldStr{original};
    
    try
    {
        std::transform(bldStr.begin(), bldStr.end(), bldStr.begin(), [](unsigned char c){return(std::tolower(c));});
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
// erasmus::workerCase::toUpper
///////////////////////////////////////////////////////////////////////////////

bool workerCase::toUpper(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    std::string bldStr{original};
    
    try
    {
        std::transform(bldStr.begin(), bldStr.end(), bldStr.begin(), [](unsigned char c){return(std::toupper(c));});
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
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerCase::~workerCase()
{

}