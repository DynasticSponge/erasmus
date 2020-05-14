//
// worker_extract.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <string>

#include "../headers/erasmus_namespace.hpp"
#include "../headers/erasmus_director.hpp"
#include "../headers/worker_extract.hpp"
#include "../headers/worker_replace.hpp"

using namespace erasmus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global variable definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus::workerExtract member definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////

workerExtract::workerExtract()
{
    this->director = nullptr;
    this->replaceWorker = nullptr;
}

workerExtract::workerExtract(erasmus::director *newDirector)
{
    this->director = newDirector;
    this->replaceWorker = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerExtract::initReplaceWorker
///////////////////////////////////////////////////////////////////////////////

bool workerExtract::initReplaceWorker()
{
    bool returnValue{true};
    if(this->replaceWorker == nullptr)
    {
        if(this->director == nullptr)
        {
            this->replaceWorker = new workerReplace();    
        }
        else
        {
            returnValue = false;
        }        
    }
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerExtract::quotedExtract
///////////////////////////////////////////////////////////////////////////////

bool workerExtract::quotedExtract(const std::string& original, std::string& revised, bool escape)
{
    bool returnValue{true};
    size_t dblQuoteStartIndex{original.find_first_of('\"')};
    if(dblQuoteStartIndex != std::string::npos)
    {
        // double quoted string should have DQUOTE as first and last char
        size_t dblQuoteEndIndex{original.find_last_of('\"')};
        if(dblQuoteStartIndex != 0 || dblQuoteEndIndex != (original.size() - 1))
        {
            returnValue = false;
        }
        else
        {
            // DQUOTE are first and last char
            // extract out the string between them
            std::string extractInit{original.substr(1, original.size() - 2)};
            
            // if escape = true then run escapeReplace on the extracted string
            if(escape)
            {
                std::string extractFinal;
                bool useReplaceWorker{this->initReplaceWorker()};
                bool replaceSuccess{true};
                
                if(useReplaceWorker)
                {
                    replaceSuccess = this->replaceWorker->escapeReplace(extractInit, extractFinal);
                }
                else
                {
                    replaceSuccess = this->director->escapeReplace(extractInit, extractFinal);
                }
                
                if(replaceSuccess)
                {
                    revised = std::move(extractFinal);
                }
                else
                {
                    returnValue = false;
                }
            }
            else
            {
                revised = std::move(extractInit);
            }
        }
    }
    else
    {
        // no double quotes in string
        returnValue = false;
    }

    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerExtract::~workerExtract()
{
    this->director = nullptr;
    if(this->replaceWorker != nullptr)
    {
        delete this->replaceWorker;
        this->replaceWorker = nullptr;
    }
}