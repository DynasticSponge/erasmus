//
// worker_url.cpp
// ~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <cctype>
#include <string>

#include "../headers/erasmus_namespace.hpp"
#include "../headers/erasmus_director.hpp"
#include "../headers/worker_numeric.hpp"
#include "../headers/worker_url.hpp"

using namespace erasmus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global variable definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus::workerURL member definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////

workerURL::workerURL()
{
    this->director = nullptr;
    this->numericWorker = nullptr;
}

workerURL::workerURL(erasmus::director *newDirector)
{
    this->director = newDirector;
    this->numericWorker = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerURL::charDecode
///////////////////////////////////////////////////////////////////////////////

bool workerURL::charDecode(const std::string& original, char& revised)
{
    bool returnValue{true};
    bool useNumericWorker{this->initNumericWorker()};

    bool goodInput{original[0] == '%'};
    if(useNumericWorker)
    {
        goodInput &= this->numericWorker->isHex(original[1]);
        goodInput &= this->numericWorker->isHex(original[2]);
    }
    else
    {
        goodInput &= this->director->isHex(original[1]);
        goodInput &= this->director->isHex(original[2]);
    }
    
    if(goodInput)
    {
        int numVal{std::stoi(original.substr(1, 2), 0, 16)};
        char outChar{static_cast<char>(numVal)};
        revised = std::move(outChar);
    }
    else
    {
        returnValue = false;
    }    

    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerURL::charEncode
///////////////////////////////////////////////////////////////////////////////

bool workerURL::charEncode(char original, std::string& revised)
{
    bool returnValue{true};
    bool useNumericWorker{this->initNumericWorker()};
    bool encodeSuccess{true};
    std::string bldStr;
    
    if(useNumericWorker)
    {
        encodeSuccess = this->numericWorker->charToHex(original, bldStr);
    }
    else
    {
        encodeSuccess = this->director->charToHex(original, bldStr);
    }

    if(encodeSuccess){
        bldStr.insert(0,"%");
        revised = std::move(bldStr);
    }
    else
    {
        returnValue = false;
    }
    
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerURL::initNumericWorker
///////////////////////////////////////////////////////////////////////////////

bool workerURL::initNumericWorker()
{
    bool returnValue{true};
    if(this->numericWorker == nullptr)
    {
        if(this->director == nullptr)
        {
            this->numericWorker = new workerNumeric();    
        }
        else
        {
            returnValue = false;
        }        
    }
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerURL::urlDecode
///////////////////////////////////////////////////////////////////////////////

bool workerURL::urlDecode(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    size_t maxBound{original.size()};
    size_t maxHexBound{(original.size() - 2)};
    std::string bldStr;
    
    for(size_t index = 0; index < maxBound && returnValue; index++)
    {
        char c0{original[index]};
        if (c0 == '%')
        {
            // verify % has at least 2 chars after it
            if(index < maxHexBound)
            {
                char decodedChar{'\0'};
                if(this->charDecode(original.substr(index,3), decodedChar))
                {
                    bldStr.push_back(decodedChar);
                    index += 2;
                }
                else
                {
                    // invalid hex chars
                    returnValue = false;
                }
            }
            else
            {
                // not enough chars after %
                returnValue = false;
            }                       
        }
        else if(c0 == '+')
        {
            // + decodes to <space>
            bldStr.push_back(' ');
        }        
        else
        {
            // character wasnt an encoded character
            bldStr.push_back(c0);
        }
    }

    // update revised with bldStr
    revised = std::move(bldStr);
    return(returnValue);   
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerURL::urlEncode
///////////////////////////////////////////////////////////////////////////////

bool workerURL::urlEncode(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    std::string bldStr;
    std::string workingStr;
    
    for(size_t index = 0; index < original.size() && returnValue; index++)
    {
        char c0{original[index]};

        // determine if c0 is url safe
        bool safeChar{static_cast<bool>(isalnum(c0))};
        safeChar |= (c0 == '-');
        safeChar |= (c0 == '.');
        safeChar |= (c0 == '_');
        safeChar |= (c0 == '~');
        
        if(!safeChar)
        {
            // encode the unsafe char
            if(this->charEncode(c0, workingStr))
            {
                // encode success, append to bldStr
                bldStr.append(workingStr);
                workingStr.clear();
            }
            else
            {
                // encode failed
                returnValue = false;
            }            
        }
        /*
        else if(c0 == ' ')
        {
            // char is <space> encode as '%20' and append to bldStr
            bldStr.append("%20");
        }
        */
        else
        {
            // safe char... append to bldStr
            bldStr.push_back(c0);
        }
    }

    if(returnValue)
    {
        // no errors, update revised
        revised = std::move(bldStr);
    }
    
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerURL::~workerURL()
{
    this->director = nullptr;
    if(this->numericWorker != nullptr)
    {
        delete this->numericWorker;
        this->numericWorker = nullptr;
    }
}