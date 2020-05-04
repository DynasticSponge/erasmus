//
// worker_base64.cpp
// ~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <bitset>

#include "../headers/erasmus_namespace.hpp"
#include "../headers/worker_base64.hpp"

using namespace erasmus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global variable definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64 member definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////////////////

workerBase64::workerBase64()
{
    this->alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64::base64Decode
///////////////////////////////////////////////////////////////////////////////

bool workerBase64::base64Decode(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    std::string bldStr;
    std::string workingString;
    this->removeWhiteSpace(original, workingString);

    std::string group;
    std::string groupBits;
    std::string outChar;
    size_t workingIndex{0};
    size_t workingSize{workingString.size()};
    size_t numGroups{this->getNumGroups(workingString, 4)};
    
    for(size_t groupIndex = 0; groupIndex < numGroups; groupIndex++)
    {
        // reuse strings
        group.clear();
        groupBits.clear();
        outChar.clear();
    
        this->getGroup(workingString, group, groupIndex, 4);
        size_t paddingIndex{group.find_first_of('=')};
        while(paddingIndex != std::string::npos)
        {
            group.erase(paddingIndex, 1);
            paddingIndex = group.find_first_of('=');
        }

        // covert each char in group to string of 6 binary digits
        size_t groupSize{group.size()};
        for(size_t letterIndex = 0; letterIndex < groupSize; letterIndex++)
        {
            size_t charIndex{this->alphabet.find_first_of(group[letterIndex])};
            groupBits.append(std::bitset<6>(charIndex).to_string());
        }

        // remove pading from groupBits based on size of group so that size of groupBits is always multiple of 8
        switch(groupSize)
        {
            case 2:
                groupBits.erase(groupBits.size() - 4, 4);
                break;
            case 3:
                groupBits.erase(groupBits.size() - 2, 2);
                break;
            default:
                break;
        }

        while(groupBits.size() > 0)
        {
            outChar.append(groupBits.substr(0, 8));
            groupBits.erase(0, 8);
            unsigned long charIndex{std::bitset<8>(outChar).to_ulong()};
            bldStr.push_back(static_cast<char>(charIndex));
            outChar.clear();
        }
    }
    revised = std::move(bldStr);
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64::base64Encode
///////////////////////////////////////////////////////////////////////////////

bool workerBase64::base64Encode(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    std::string bldStr;
    
    std::string group;
    std::string groupBits;
    std::string groupAppend;
    std::string outChar;
    size_t originalIndex{0};
    size_t originalSize{original.size()};
    size_t numGroups{this->getNumGroups(original, 3)};
    
    // loop through each group
    for(size_t groupIndex = 0; groupIndex < numGroups; groupIndex++)
    {
        // reuse strings
        group.clear();
        groupBits.clear();
        groupAppend.clear();
        outChar.clear();

        this->getGroup(original, group, groupIndex, 3);
        
        // covert each letter in group to string of 8 binary digits
        size_t groupSize{group.size()};
        for(size_t letterIndex = 0; letterIndex < groupSize; letterIndex++)
        {
            groupBits.append(std::bitset<8>(group[letterIndex]).to_string());
        }

        // pad out groupBits based on size of group so that size of groupBits is always multiple of 6
        switch(groupSize)
        {
            case 1:
                groupBits.append("0000");
                groupAppend = "==";
                break;
            case 2:
                groupBits.append("00");
                groupAppend = "=";
                break;
            default:
                break;
        }

        // lookup alphabet entry using index = extract out 6bit groups fom groupBits and convert to decimal 
        while(groupBits.size() > 0)
        {
            outChar.append("00");
            outChar.append(groupBits.substr(0,6));
            groupBits.erase(0, 6);
            unsigned long charIndex{std::bitset<8>(outChar).to_ulong()};
            bldStr.push_back(this->alphabet[charIndex]);
            outChar.clear();
        }

        // append padding chars if necessary
        if(groupAppend.size() > 0)
        {
            bldStr.append(groupAppend);
        }
    }
    revised = std::move(bldStr);
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64::getGroup
///////////////////////////////////////////////////////////////////////////////

void workerBase64::getGroup(const std::string& original, std::string& group, size_t grpIndex, size_t grpSize)
{
    // extract charcters in current group from original string
    group.clear();
    size_t originalIndex{grpIndex * grpSize};
    if(originalIndex > (original.size() - grpSize))
    {
        group = original.substr(originalIndex);
    }
    else
    {
        group = original.substr(originalIndex, grpSize);
    }        
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64::getNumGroups
///////////////////////////////////////////////////////////////////////////////

size_t workerBase64::getNumGroups(const std::string& original, size_t groupSize)
{
    // determing number of groups to convert
    size_t originalSize{original.size()};
    size_t numGroups{0};
    if((originalSize % groupSize) > 0)
    {
        numGroups = originalSize / groupSize + 1;
    }
    else
    {
        numGroups = originalSize / groupSize;
    }
    return(numGroups);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64::mimeEncode
///////////////////////////////////////////////////////////////////////////////

bool workerBase64::mimeEncode(const std::string& original, std::string& revised)
{
    bool returnValue{true};
    std::string newLine{"\r\n"};
    std::string mimeString;
    std::string base64String;
    if(this->base64Encode(original, base64String))
    {
        while(base64String.size() > 0)
        {
            if(base64String.size() >= 76)
            {
                mimeString.append(base64String.substr(0, 76));
                base64String.erase(0, 76);
            }
            else
            {
                mimeString.append(base64String);
                base64String.clear();
            }
            mimeString.append(newLine);            
        }
    }
    else
    {
        returnValue = false;
    }
    if(returnValue)
    {
        revised = std::move(mimeString);
    }    
    return(returnValue);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerBase64::getGroup
///////////////////////////////////////////////////////////////////////////////

void workerBase64::removeWhiteSpace(const std::string& original, std::string& revised)
{
    std::string wspChars{" \f\n\r\t\v"};
    std::string workingString{original};
    size_t wspIndex{workingString.find_first_of(wspChars)};
    while(wspIndex != std::string::npos)
    {
        workingString.erase(wspIndex, 1);
        wspIndex = workingString.find_first_of(wspChars);
    }
    revised = std::move(workingString);
    return;
}

///////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerBase64::~workerBase64()
{

}