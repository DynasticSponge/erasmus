//
// worker_deflate.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <bitset>
#include <chrono>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "../headers/erasmus_namespace.hpp"
#include "../headers/data_match.hpp"
#include "../headers/data_node.hpp"
#include "../headers/worker_deflate.hpp"

using namespace erasmus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global variable definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate member definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////

workerDeflate::workerDeflate(std::string original)
{
    this->source = std::move(original);
    this->original = std::string_view(this->source);
    this->initDistCodes();
    this->initLitCodes();
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::buildMatches
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::buildMatches()
{
    size_t i{3};
    size_t originalSize{this->original.size()};
    
    std::map<std::string_view, size_t> matchMap;
    for(size_t index = 0; index <= (originalSize - i); index++)
    {
        std::string_view window{this->original.substr(index, i)};
        if(matchMap.count(window) == 0)
        {
            matchMap[window] = index;
        }
        else
        {
            if(index - matchMap[window] <= 32768)
            {
                dataMatch *newMatch{new dataMatch(matchMap[window], index, i)};
                this->matchList[index] = newMatch;
                this->growMatch(index);    
            }
            matchMap[window] = index;
        }                
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::cullMatches
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::cullMatches()
{   
    auto matchIter{this->matchList.begin()};
    while(matchIter != this->matchList.end())
    {
        auto searchIter{std::next(matchIter, 1)};
        if(searchIter != this->matchList.end())
        {
            //size_t maxNextStart{matchIter->first + (matchIter->second->getLength() + 2)};
            //size_t maxNextStart{matchIter->first + (matchIter->second->getLength() + 1)};
            size_t maxNextStart{matchIter->first + (matchIter->second->getLength())};
            size_t nextStart{matchIter->first};
            size_t maxEnd{matchIter->first + (matchIter->second->getLength() - 1)};
            while(searchIter->first <= maxNextStart){
                size_t curEnd{searchIter->first + (searchIter->second->getLength() - 1)};
                if(curEnd > maxEnd)
                {
                    maxEnd = curEnd;
                    nextStart = searchIter->first;
                    searchIter++;
                }
                else
                {
                    delete searchIter->second;
                    searchIter = this->matchList.erase(searchIter);
                }
                if(searchIter == this->matchList.end())
                {
                    break;
                }
            }
            searchIter--;
            while(searchIter != matchIter)
            {
                if(searchIter->first != nextStart)
                {
                    delete searchIter->second;
                    searchIter = this->matchList.erase(searchIter);
                }
                searchIter--;
            }
            searchIter++;
            if(searchIter != this->matchList.end())
            {
                size_t saveIndex{matchIter->first};
                matchIter = this->trimTwoFer(matchIter);
                if(matchIter != this->matchList.end())
                {
                    if(matchIter->first == saveIndex)
                    {
                        matchIter++;
                    }
                }
                else
                {
                    break;    
                }
            }
            else
            {
                matchIter++;
            }
        }
        else
        {
            matchIter++;
        }       
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::deflate
///////////////////////////////////////////////////////////////////////////////

std::string workerDeflate::deflate()
{
    std::string returnValue;
    nowPoint tStart{std::chrono::steady_clock::now()};
    
    // create modified data sequence of chars and matches broken into blocks
    this->buildMatches();
    this->cullMatches();
    this->generateModified();
    size_t blocks{this->modified.size()};
    
    // loop through each block to build its code trees and generate bit output
    for(size_t block = 0; block < blocks; block++)
    {
        this->initDistBitCodes(block);
        this->initLenBitCodes(block); 
        this->initLitBitCodes(block);
        this->sortLitCodes(block);
        this->generateTree();
        this->generateLitBitCodeLens(block, *(this->codeBuilder.begin()), 0);
        this->generateLitBitCodes(block);
        this->generateHLit(block);
        this->sortDistCodes(block);
        this->generateTree();
        this->generateDistBitCodeLens(block, *(this->codeBuilder.begin()), 0);
        this->generateDistBitCodes(block);
        this->generateHDist(block);
        this->generateHLitDistModified(block);
        this->sortLenCodes(block);
        this->generateTree();
        this->generateLenBitCodeLens(block, *(this->codeBuilder.begin()), 0);
        this->generateLenBitCodes(block);
        this->generateHCLenModified(block);
        this->generateHCLen(block);
    }

    // loop through each block to output raw bits
    std::string temp;
    for(size_t block = 0; block < blocks; block++)
    {
        if(block == (blocks - 1))
        {
            this->destBits.append("1");
        }
        else
        {
            this->destBits.append("0");
        }

        temp.clear();
        temp = std::bitset<2>(2).to_string();
        std::reverse(temp.begin(), temp.end());
        this->destBits.append(temp);
        
        temp.clear();
        temp = std::bitset<5>(this->hlit[block] - 257).to_string();
        std::reverse(temp.begin(), temp.end());
        this->destBits.append(temp);
        temp.clear();
        temp = std::bitset<5>(this->hdist[block] - 1).to_string();
        std::reverse(temp.begin(), temp.end());
        this->destBits.append(temp);
        temp.clear();
        temp = std::bitset<4>(this->hclen[block] - 4).to_string();
        std::reverse(temp.begin(), temp.end());
        this->destBits.append(temp);
        for(auto index = 0; index < this->hclen[block]; index++)
        {
            temp.clear();
            temp = std::bitset<3>(this->hclenModified[block][index].first).to_string();
            std::reverse(temp.begin(), temp.end());
            this->destBits.append(temp);
        }
        this->outputHLitDistModified(block);
        this->outputModified(block);
    }
    
    returnValue = this->outputDestination();
    nowPoint tEnd{std::chrono::steady_clock::now()};
    this->dumpTree();
    this->reportTime(tStart, tEnd);
    return(std::move(returnValue));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpDivider
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::dumpDivider()
{
    auto prevFill = std::cout.fill('-');
    std::cout.width(120);
    std::cout << '-';
    std::cout << std::endl;
    std::cout.fill(prevFill);
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpHeader
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::dumpHeader(size_t spanID, size_t index)
{
    std::cout.setf(std::ios::left, std::ios::adjustfield);
    std::string tempStr;

    std::cout.width(16);
    tempStr.append("|   Span-");
    tempStr.append(std::to_string(spanID));
    std::cout << tempStr;

    std::cout.width(12);
    tempStr.clear();
    tempStr.append("S:");
    tempStr.append(std::to_string(index));
    std::cout << tempStr;

    std::cout.width(36);
    tempStr.clear();
    tempStr.append(" ");
    std::cout << tempStr;
    
    std::cout.width(56);
    tempStr.clear();
    tempStr.append("| Coverage Map");
    std::cout << tempStr;

    tempStr.clear();
    tempStr.append("|");
    std::cout << tempStr << std::endl;
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpMatch
///////////////////////////////////////////////////////////////////////////////

size_t workerDeflate::dumpMatch(size_t spanIndex, size_t index, size_t count)
{
    bool returnValue{false};
    std::string tempString;
    
    dataMatch *curMatch{this->matchList[index]};
    size_t spanEnd{spanIndex + 54};
    size_t curDistance = curMatch->getDistance();
    size_t curLength = curMatch->getLength();
    size_t curEnd = index + (curLength - 1);
        
    std::cout.width(16);
    tempString.clear();
    tempString.append("|   Match-");
    tempString.append(std::to_string(count));
    std::cout << tempString;

    std::cout.width(12);
    tempString.clear();
    tempString.append("S:");
    tempString.append(std::to_string(index));
    std::cout << tempString;

    std::cout.width(12);
    tempString.clear();
    tempString.append("E:");
    tempString.append(std::to_string(curEnd));
    std::cout << tempString;

    std::cout.width(12);
    tempString.clear();
    tempString.append("D:");
    tempString.append(std::to_string(curDistance));
    std::cout << tempString;

    std::cout.width(12);
    tempString.clear();
    tempString.append("L:");
    tempString.append(std::to_string(curLength));      
    std::cout << tempString;

    std::cout.width(56);
    tempString.clear();
    tempString.append("|");

    if(curEnd > spanEnd)
    {
        curEnd = spanEnd;
    }
    size_t charIndex{spanIndex};
    while(charIndex < index)
    {
        tempString.append(" ");
        charIndex++;
    }
    while(charIndex <= curEnd)
    {
        tempString.append("*");
        charIndex++;
    }
    while(charIndex <= spanEnd)
    {
        tempString.append(" ");
        charIndex++;
    }
    tempString.append("|");
    std::cout << tempString << std::endl;
    return(curEnd);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpMatchList
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::dumpMatchList()
{
    size_t matchCount{0};
    size_t matchEnd{0};
    size_t spanCount{0};
    size_t spanIndex{0};
    size_t spanEnd{0};
    bool spanActive{false};
    while(this->matchList.size() > 0)
    {
        auto matchIter{this->matchList.begin()};
        
        if(matchIter->first > spanEnd && spanCount != 0)
        {
            spanActive = false;
            this->dumpDivider();
            std::cout << std::endl << std::endl;
        }

        if(!spanActive )
        {
            spanIndex = matchIter->first;
            spanEnd = spanIndex + 54;
            matchCount = 0;
            this->dumpDivider();
            this->dumpHeader(spanCount, spanIndex);
            this->dumpDivider();
            spanActive = true;
            spanCount++;
        }
        
        this->dumpMatch(spanIndex, matchIter->first, matchCount);
        matchCount++;
        
        this->matchList.erase(matchIter);
    }
    this->dumpDivider();
    std::cout << std::endl << std::endl;
    this->dumpDivider();
    this->dumpDivider();
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpTree
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::dumpTree()
{
    std::cout.setf(std::ios::left, std::ios::adjustfield);
    std::string tempStr;
    
    for(auto block = 0; block < this->modified.size(); block++)
    {
        std::cout << std::endl;
        this->dumpDivider();
        tempStr.clear();
        tempStr = "Literal/Length Bit Codes:  Block: ";
        tempStr.append(std::to_string(block));
        tempStr.append("    hlit: ");
        tempStr.append(std::to_string(this->hlit[block]));
        std::cout << tempStr << std::endl;
        this->dumpDivider();
        std::cout << std::endl;
        
        for(size_t i = 0; i < 286; i++)
        {
            if(this->litNumCounts[block][i] == 0)
            {
                continue;
            }

            this->dumpTreeLineLead(tempStr, block, i, 0);
            this->dumpTreeLineBits(tempStr,this->litBitCodes[block][i].first, this->litBitCodes[block][i].second);
            std::cout << std::endl;
        }

        std::cout << std::endl;
        this->dumpDivider();
        tempStr.clear();
        tempStr = "Distance Bit Codes:  Block: ";
        tempStr.append(std::to_string(block));
        tempStr.append("    hdist: ");
        tempStr.append(std::to_string(this->hdist[block]));
        std::cout << tempStr << std::endl;
        this->dumpDivider();
        std::cout << std::endl;
        
        for(code_t i = 0; i < 30; i++)
        {
            if(this->distNumCounts[block][i] == 0)
            {
                continue;
            }
            
            this->dumpTreeLineLead(tempStr, block, i, 1);
            this->dumpTreeLineBits(tempStr,this->distBitCodes[block][i].first, this->distBitCodes[block][i].second);
            std::cout << std::endl;
        }

        std::cout << std::endl;
        this->dumpDivider();
        tempStr.clear();
        tempStr = "CodeLength Bit Codes:  Block: ";
        tempStr.append(std::to_string(block));
        tempStr.append("    hclen: ");
        tempStr.append(std::to_string(this->hclen[block]));
        std::cout << tempStr << std::endl;
        this->dumpDivider();
        std::cout << std::endl;
        
        for(code_t i = 0; i < 19; i++)
        {
            if(this->hclenCounts[block][i] == 0)
            {
                continue;
            }
            
            this->dumpTreeLineLead(tempStr, block, i, 2);
            this->dumpTreeLineBits(tempStr,this->lenBitCodes[block][i].first, this->lenBitCodes[block][i].second);
            std::cout << std::endl;
        }
    }
        
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpTreeLineBits
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::dumpTreeLineBits(std::string &tempStr, code_t codeLen, std::string code)
{
    std::cout.width(28);
    tempStr.clear();
    tempStr.append("BitCode: ");
    tempStr.append(code);
    std::cout << tempStr;

    std::cout.width(32);
    tempStr.clear();
    tempStr.append("Bit Length: ");
    tempStr.append(std::to_string(codeLen));
    std::cout << tempStr;
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::dumpTreeLineLead
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::dumpTreeLineLead(std::string &tempStr, size_t block, size_t i, code_t flag)
{
    std::cout.width(16);
    tempStr.clear();
    tempStr.append("Code: ");
    tempStr.append(std::to_string(i));
    std::cout << tempStr;

    std::cout.width(16);
    tempStr.clear();
    tempStr.append("Count: ");
    switch(flag)
    {
        case 0:
            tempStr.append(std::to_string(this->litNumCounts[block][i]));
            break;
        case 1:
            tempStr.append(std::to_string(this->distNumCounts[block][i]));
            break;
        case 2:
            tempStr.append(std::to_string(this->hclenCounts[block][i]));
            break;
        default:
            break;
    }
    std::cout << tempStr;
}



///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateDistBitCodeLens
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateDistBitCodeLens(size_t block, const dataNode& curNode, code_t parent)
{
    if(curNode.hasKids > 0)
    {   
        code_t newParent{parent};
        newParent++;
        this->generateDistBitCodeLens(block, curNode.kids[0], newParent);
        if(curNode.hasKids > 1)
        {
            this->generateDistBitCodeLens(block, curNode.kids[1], newParent);
        }        
    }
    else
    {
        this->distBitCodes[block][curNode.code].first = parent;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateDistBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateDistBitCodes(size_t block)
{
    code_t code{0};
    code_t maxLen{0};
    std::map<code_t, code_t> codeLenCounts;
    std::map<code_t, code_t> nextCode;
    for(code_t i = 0; i < 16; i++)
    {
        codeLenCounts[i] = 0;
    }
    for(code_t i = 0; i < this->distBitCodes[block].size(); i++)
    {
        if(this->distBitCodes[block][i].first == 0)
        {
            continue;
        }
        if(this->distBitCodes[block][i].first > maxLen)
        {
            maxLen = this->distBitCodes[block][i].first;
        }
        codeLenCounts[this->distBitCodes[block][i].first]++;
    }
    for (code_t len = 1; len <= maxLen; len++) {
        code = (code + codeLenCounts[(len - 1)]) << 1;
        nextCode[len] = code;
    }
    for (code_t curCode = 0;  curCode <= 286; curCode++)
    {
        code_t codeLen{this->distBitCodes[block][curCode].first};

        if (codeLen != 0)
        {
            code = nextCode[codeLen];
            this->distBitCodes[block][curCode].second = this->getBitCodeString(codeLen, code);
            nextCode[codeLen]++;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateHCLen
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateHCLen(size_t block)
{
    if(this->hclen.size() < (block + 1))
    {
        this->hclen.push_back(19);
    }
    for(this->hclen[block] = 19; this->hclen[block] > 0; this->hclen[block]--)
    {
        if(this->hclenModified[block][(this->hclen[block] - 1)].first > 0)
        {
            break;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateHCLenModified
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateHCLenModified(size_t block)
{
    if(this->hclenModified.size() < (block + 1))
    {
        std::vector<codePair> newVector;
        this->hclenModified.push_back(std::move(newVector));
    }
    this->hclenModified[block].push_back({this->lenBitCodes[block][16].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][17].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][18].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][0].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][8].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][7].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][9].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][6].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][10].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][5].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][11].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][4].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][12].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][3].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][13].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][2].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][14].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][1].first,0});
    this->hclenModified[block].push_back({this->lenBitCodes[block][15].first,0});
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateHDist
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateHDist(size_t block)
{
    if(this->hdist.size() < (block + 1))
    {
        this->hdist.push_back(30);
    }
    for(this->hdist[block] = 30; this->hdist[block] > 0; this->hdist[block]--)
    {
        if(this->distBitCodes[block][(this->hdist[block] - 1)].first > 0)
        {
            break;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateHLitDistModified
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateHLitDistModified(size_t block)
{
    if(this->hLitDistModified.size() < (block + 1))
    {
        std::vector<codePair> newVector;
        this->hLitDistModified.push_back(std::move(newVector));
    }
    if(this->hclenCounts.size() < (block + 1))
    {
        std::map<code_t, code_t> newMap;
        this->hclenCounts.push_back(std::move(newMap));
        for(code_t i = 0; i < 19; i++)
        {
            this->hclenCounts[block][i] = 0;
        }
    }

    code_t curCode;
    code_t lastCode{20};
    code_t repeatCount{0};

    for(code_t i = 0; i < this->hlit[block]; i++)
    {
        curCode = this->litBitCodes[block][i].first;
        if(curCode == lastCode)
        {
            repeatCount++;
        }
        else
        {
            this->updateHLitDistModified(block, curCode, lastCode, repeatCount);
        }
    }
    for(code_t i = 0; i < this->hdist[block]; i++)
    {
        curCode = this->distBitCodes[block][i].first;
        if(curCode == lastCode)
        {
            repeatCount++;
        }
        else
        {
            this->updateHLitDistModified(block, curCode, lastCode, repeatCount);
        }
    }
    if(lastCode == 0 || repeatCount > 0)
    {
        this->updateHLitDistModified(block, curCode, lastCode, repeatCount);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateHLit
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateHLit(size_t block)
{
    if(this->hlit.size() < (block + 1))
    {
        this->hlit.push_back(286);    
    }
    for(this->hlit[block] = 286; this->hlit[block] > 0; this->hlit[block]--)
    {
        if(this->litBitCodes[block][(this->hlit[block] - 1)].first > 0)
        {
            break;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateLenBitCodeLens
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateLenBitCodeLens(size_t block, const dataNode& curNode, code_t parent)
{
    if(curNode.hasKids > 0)
    {   
        code_t newParent{parent};
        newParent++;    
        this->generateLenBitCodeLens(block, curNode.kids[0], newParent);
        if(curNode.hasKids > 1)
        {
            this->generateLenBitCodeLens(block, curNode.kids[1], newParent);
        }
    }
    else
    {
        this->lenBitCodes[block][curNode.code].first = parent;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateLenBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateLenBitCodes(size_t block)
{
    code_t code{0};
    code_t maxLen{0};
    std::map<code_t, code_t> codeLenCounts;
    std::map<code_t, code_t> nextCode;
    for(code_t i = 0; i < 16; i++)
    {
        codeLenCounts[i] = 0;
    }
    for(code_t i = 0; i < this->lenBitCodes[block].size(); i++)
    {
        if(this->lenBitCodes[block][i].first == 0)
        {
            continue;
        }
        if(this->lenBitCodes[block][i].first > maxLen)
        {
            maxLen = this->lenBitCodes[block][i].first;
        }
        codeLenCounts[this->lenBitCodes[block][i].first]++;
    }
    for (code_t len = 1; len <= maxLen; len++) {
        code = (code + codeLenCounts[(len - 1)]) << 1;
        nextCode[len] = code;
    }
    for (code_t curCode = 0;  curCode < 19; curCode++)
    {
        code_t codeLen{this->lenBitCodes[block][curCode].first};

        if (codeLen != 0)
        {
            code = nextCode[codeLen];
            this->lenBitCodes[block][curCode].second = this->getBitCodeString(codeLen, code);
            nextCode[codeLen]++;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateLitBitCodeLens
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateLitBitCodeLens(size_t block, const dataNode& curNode, code_t parent)
{
    if(curNode.hasKids > 0)
    {   
        code_t newParent{parent};
        newParent++;    
        this->generateLitBitCodeLens(block, curNode.kids[0], newParent);
        if(curNode.hasKids > 1)
        {
            this->generateLitBitCodeLens(block, curNode.kids[1], newParent);
        }
    }
    else
    {
        this->litBitCodes[block][curNode.code].first = parent;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateLitBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateLitBitCodes(size_t block)
{
    code_t code{0};
    code_t maxLen{0};
    std::map<code_t, code_t> codeLenCounts;
    std::map<code_t, code_t> nextCode;
    for(code_t i = 0; i < 16; i++)
    {
        codeLenCounts[i] = 0;
    }
    for(code_t i = 0; i < this->litBitCodes[block].size(); i++)
    {
        if(this->litBitCodes[block][i].first == 0)
        {
            continue;
        }
        if(this->litBitCodes[block][i].first > maxLen)
        {
            maxLen = this->litBitCodes[block][i].first;
        }
        codeLenCounts[this->litBitCodes[block][i].first]++;
    }
    for (code_t len = 1; len <= maxLen; len++) {
        code = (code + codeLenCounts[(len - 1)]) << 1;
        nextCode[len] = code;
    }
    for (code_t curCode = 0;  curCode < 286; curCode++)
    {
        code_t codeLen{this->litBitCodes[block][curCode].first};

        if (codeLen != 0)
        {
            code = nextCode[codeLen];
            this->litBitCodes[block][curCode].second = this->getBitCodeString(codeLen, code);
            nextCode[codeLen]++;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateModified
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateModified()
{
    size_t sourceIndex{0};
    size_t blockIndex{0};
    std::vector<codePair> block;
    std::map<code_t, code_t> litLenCounts;
    for(code_t i=0; i < 285; i++)
    {
        litLenCounts[i] = 0;
    }
    std::map<code_t, code_t> distCounts;
    for(code_t i=0; i < 30; i++)
    {
        distCounts[i] = 0;
    }
    codePair newCode;
    while(sourceIndex < this->source.size())
    {
        if(this->matchList.count(sourceIndex) > 0)
        {
            size_t length = this->matchList[sourceIndex]->getLength();
            newCode = this->getLitCode(length);
            block.push_back(std::move(newCode));
            litLenCounts[newCode.first] += 1;
            newCode = this->getDistCode(this->matchList[sourceIndex]->getDistance());
            block.push_back(std::move(newCode));
            distCounts[newCode.first] += 1;
            sourceIndex += length;
            blockIndex += length;
            delete this->matchList[length];
            this->matchList.erase(length);
        }
        else
        {
            newCode.first = static_cast<code_t>(static_cast<unsigned char>(this->source[sourceIndex]));
            newCode.second = 0;
            block.push_back(std::move(newCode));
            litLenCounts[newCode.first] += 1;
            sourceIndex++;
            blockIndex++;
        }

        bool endBlock{false};
        if(this->matchList.count(sourceIndex) > 0)
        {
            size_t length = this->matchList[sourceIndex]->getLength();
            if(blockIndex + length >= 32767)
            {
                endBlock = true;
            }
        }
        else
        {
            if(blockIndex >= 32767)
            {
                endBlock = true;
            }
        }
        
        if(endBlock)
        {
            newCode.first = 256;
            newCode.second = 0;
            block.push_back(std::move(newCode));
            litLenCounts[newCode.first] += 1;
            this->modified.push_back(std::move(block));
            this->litNumCounts.push_back(std::move(litLenCounts));
            this->distNumCounts.push_back(std::move(distCounts));
            distCounts.clear();
            for(code_t i=0; i < 30; i++)
            {
                distCounts[i] = 0;
            }
            litLenCounts.clear();
            for(code_t i=0; i < 285; i++)
            {
                litLenCounts[i] = 0;
            }
            block.clear();            
            blockIndex = 0;
        }
    }
    newCode.first = 256;
    newCode.second = 0;
    block.push_back(std::move(newCode));
    litLenCounts[newCode.first] += 1;
    this->modified.push_back(std::move(block));
    this->litNumCounts.push_back(std::move(litLenCounts));
    this->distNumCounts.push_back(std::move(distCounts));
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateTree
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateTree()
{
    bool finished{this->codeBuilder.empty()};
    while(!finished)
    {
        dataNode node1 = dataNode();
        dataNode node2 = dataNode();
        auto frontIter = this->codeBuilder.begin();
        while(node1.weight == 0 && frontIter != this->codeBuilder.end())
        {
            node1 = *frontIter;
            frontIter = this->codeBuilder.erase(frontIter);
        }
        if(this->codeBuilder.empty())
        {
            if(node1.weight != 0)
            {
                if(node1.hasKids > 0){
                    this->codeBuilder.insert(node1);
                }
                else
                {
                    dataNode newNode = dataNode();
                    dataNode newNode2 = dataNode();
                    newNode2.kids.push_back(node1);
                    newNode2.hasKids = 1;
                    newNode.kids.push_back(newNode2);
                    newNode.hasKids = 1;
                    this->codeBuilder.insert(newNode);
                }               
            }
            finished = true;
        }
        if(!finished)
        {
            while(node2.weight == 0 && frontIter != this->codeBuilder.end())
            {
                node2 = *frontIter;
                frontIter = this->codeBuilder.erase(frontIter);
            }
            if(node2.weight == 0)
            {
                finished = true;
            }
        }
        if(!finished)
        {
            dataNode newNode;
            if(node1.code < node2.code)
            {
                newNode.code = node1.code;
            }
            else
            {
                newNode.code = node2.code;
            }
            newNode.weight = node1.weight + node2.weight;
            if(node1.weight < node2.weight)
            {
                newNode.kids.push_back(std::move(node2));
                newNode.kids.push_back(std::move(node1));
            }
            else if(node1.weight == node2.weight)
            {
                if(node1.code < node2.code)
                {
                    newNode.kids.push_back(std::move(node1));
                    newNode.kids.push_back(std::move(node2));
                }
                else
                {
                    newNode.kids.push_back(std::move(node2));
                    newNode.kids.push_back(std::move(node1));
                }
            }
            else
            {
                newNode.kids.push_back(std::move(node1));
                newNode.kids.push_back(std::move(node2));
            }
            newNode.hasKids = 2;
            this->codeBuilder.insert(newNode);
        }     
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::getBitCodeString
///////////////////////////////////////////////////////////////////////////////

std::string workerDeflate::getBitCodeString(code_t codeLen, code_t code)
{
    std::string returnString;
    switch(codeLen)
    {
        case 1:
            returnString = std::bitset<1>(code).to_string();
            break;
        case 2:
            returnString = std::bitset<2>(code).to_string();
            break;
        case 3:
            returnString = std::bitset<3>(code).to_string();
            break;
        case 4:
            returnString = std::bitset<4>(code).to_string();
            break;
        case 5:
            returnString = std::bitset<5>(code).to_string();
            break;
        case 6:
            returnString = std::bitset<6>(code).to_string();
            break;
        case 7:
            returnString = std::bitset<7>(code).to_string();
            break;
        case 8:
            returnString = std::bitset<8>(code).to_string();
            break;
        case 9:
            returnString = std::bitset<9>(code).to_string();
            break;
        case 10:
            returnString = std::bitset<10>(code).to_string();
            break;
        case 11:
            returnString = std::bitset<11>(code).to_string();
            break;
        case 12:
            returnString = std::bitset<12>(code).to_string();
            break;
        case 13:
            returnString = std::bitset<13>(code).to_string();
            break;
        case 14:
            returnString = std::bitset<14>(code).to_string();
            break;
        case 15:
            returnString = std::bitset<15>(code).to_string();
            break;
        default:
            break;
    }
    return(returnString);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::getDistCode
///////////////////////////////////////////////////////////////////////////////

codePair workerDeflate::getDistCode(size_t distance)
{
    codePair returnCode;
    bool codeFound{false};
    size_t prevMax{0};
    auto iter{this->distNumCodes.begin()};
    while(!codeFound)
    {
        if(distance > iter->first)
        {
            prevMax = iter->first + 1;
            iter++;
        }
        else
        {
            auto prevIter{iter};
            prevIter--;
            returnCode.first = static_cast<code_t>(iter->second);
            returnCode.second = static_cast<code_t>(distance - prevMax);
            codeFound = true;
        }        
    }
    return(returnCode);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::getLitCode
///////////////////////////////////////////////////////////////////////////////

codePair workerDeflate::getLitCode(size_t length)
{
    codePair returnCode;
    bool codeFound{false};
    size_t prevMax{0};
    auto iter{this->litNumCodes.begin()};
    while(!codeFound)
    {
        if(length > iter->first)
        {
            prevMax = iter->first + 1;
            iter++;
        }
        else
        {
            returnCode.first = static_cast<code_t>(iter->second);
            returnCode.second = static_cast<code_t>(length - prevMax);
            codeFound = true;
        }        
    }
    return(returnCode);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::growMatch
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::growMatch(size_t matchIndex)
{
    size_t prevIndex{this->matchList[matchIndex]->getPrevious()};
    size_t resize{0};
    bool matched{true};
    for(size_t i=3; i < 258 && matched; i++)
    {
        if(this->original[prevIndex + i] == this->original[matchIndex + i])
        {
            resize++;
        }
        else
        {
            matched = false;
        }            
    }
    this->matchList[matchIndex]->postBack(resize);
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::inflate
///////////////////////////////////////////////////////////////////////////////

std::string workerDeflate::inflate()
{
    std::string returnString;
    return(std::move(returnString));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initDistCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initDistCodes()
{
    this->distNumCodes.insert({1,0});
    this->distNumCodes.insert({2,1});
    this->distNumCodes.insert({3,2});
    this->distNumCodes.insert({4,3});
    this->distNumCodes.insert({6,4});
    this->distNumCodes.insert({8,5});
    this->distNumCodes.insert({12,6});
    this->distNumCodes.insert({16,7});
    this->distNumCodes.insert({24,8});
    this->distNumCodes.insert({32,9});
    this->distNumCodes.insert({48,10});
    this->distNumCodes.insert({64,11});
    this->distNumCodes.insert({96,12});
    this->distNumCodes.insert({128,13});
    this->distNumCodes.insert({192,14});
    this->distNumCodes.insert({256,15});
    this->distNumCodes.insert({384,16});
    this->distNumCodes.insert({512,17});
    this->distNumCodes.insert({768,18});
    this->distNumCodes.insert({1024,19});
    this->distNumCodes.insert({1536,20});
    this->distNumCodes.insert({2048,21});
    this->distNumCodes.insert({3072,22});
    this->distNumCodes.insert({4096,23});
    this->distNumCodes.insert({6144,24});
    this->distNumCodes.insert({8192,25});
    this->distNumCodes.insert({12288,26});
    this->distNumCodes.insert({16384,27});
    this->distNumCodes.insert({24576,28});
    this->distNumCodes.insert({32768,29});
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initDistBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initDistBitCodes(size_t block)
{
    if(this->distBitCodes.size() < (block + 1))
    {
        std::map<code_t, std::pair<code_t, std::string>> newMap;
        this->distBitCodes.push_back(std::move(newMap));
    }
    else
    {
        this->distBitCodes[block].clear();
    }    
    
    for(code_t i = 0; i < 30; i++)
    {
        this->distBitCodes[block][i] = {0,""};
    }
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initLenBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initLenBitCodes(size_t block)
{
    if(this->lenBitCodes.size() < (block + 1))
    {
        std::map<code_t, std::pair<code_t, std::string>> newMap;
        this->lenBitCodes.push_back(std::move(newMap));
    }
    else
    {
        this->lenBitCodes[block].clear();
    }

    for(code_t i = 0; i < 19; i++)
    {
        this->lenBitCodes[block][i] = {0,""};
    }
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initLitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initLitCodes()
{
    this->litNumCodes.insert({3,257});
    this->litNumCodes.insert({4,258});
    this->litNumCodes.insert({5,259});
    this->litNumCodes.insert({6,260});
    this->litNumCodes.insert({7,261});
    this->litNumCodes.insert({8,262});
    this->litNumCodes.insert({9,263});
    this->litNumCodes.insert({10,264});
    this->litNumCodes.insert({12,265});
    this->litNumCodes.insert({14,266});
    this->litNumCodes.insert({16,267});
    this->litNumCodes.insert({18,268});
    this->litNumCodes.insert({22,269});
    this->litNumCodes.insert({26,270});
    this->litNumCodes.insert({30,271});
    this->litNumCodes.insert({34,272});
    this->litNumCodes.insert({42,273});
    this->litNumCodes.insert({50,274});
    this->litNumCodes.insert({58,275});
    this->litNumCodes.insert({66,276});
    this->litNumCodes.insert({82,277});
    this->litNumCodes.insert({98,278});
    this->litNumCodes.insert({114,279});
    this->litNumCodes.insert({130,280});
    this->litNumCodes.insert({162,281});
    this->litNumCodes.insert({194,282});
    this->litNumCodes.insert({226,283});
    this->litNumCodes.insert({257,284});
    this->litNumCodes.insert({258,285});
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initLitBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initLitBitCodes(size_t block)
{
    if(this->litBitCodes.size() < (block + 1))
    {
        std::map<code_t, std::pair<code_t, std::string>> newMap;
        this->litBitCodes.push_back(std::move(newMap));
    }
    else
    {
        this->litBitCodes[block].clear();
    }

    for(code_t i = 0; i < 286; i++)
    {
        this->litBitCodes[block][i] = {0,""};
    }
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::outputDestination
///////////////////////////////////////////////////////////////////////////////

std::string workerDeflate::outputDestination()
{
    std::string returnString;
    std::string workStr;
    while(this->destBits.size() > 0)
    {
        workStr.clear();
        if(this->destBits.size() < 8)
        {
            workStr.append(this->destBits);
            this->destBits.clear();
        }
        else
        {
            workStr.append(this->destBits.substr(0,8));
            this->destBits.erase(0,8);
        }
        std::reverse(workStr.begin(), workStr.end());
        unsigned long newLong;
        switch(workStr.size())
        {
            case 1:
                newLong = std::bitset<1>(workStr).to_ulong();
                break;
            case 2:
                newLong = std::bitset<2>(workStr).to_ulong();
                break;
            case 3:
                newLong = std::bitset<3>(workStr).to_ulong();
                break;
            case 4:
                newLong = std::bitset<4>(workStr).to_ulong();
                break;
            case 5:
                newLong = std::bitset<5>(workStr).to_ulong();
                break;
            case 6:
                newLong = std::bitset<6>(workStr).to_ulong();
                break;
            case 7:
                newLong = std::bitset<7>(workStr).to_ulong();
                break;
            case 8:
                newLong = std::bitset<8>(workStr).to_ulong();
                break;
            default:
                break;
        }
        returnString.push_back(static_cast<char>(newLong));        
    }
    return(std::move(returnString));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::outputDistCode
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::outputDistCode(const std::string& codeBits, code_t code, code_t remain)
{
    bool hasRemain{false};
    std::string remainBits;
    switch(code)
    {
        case 4:
            [[fallthrough]];
        case 5:
            remainBits = std::bitset<1>(remain).to_string();
            hasRemain = true;
            break;
        case 6:
            [[fallthrough]];
        case 7:
            remainBits = std::bitset<2>(remain).to_string();
            hasRemain = true;
            break;
        case 8:
            [[fallthrough]];
        case 9:
            remainBits = std::bitset<3>(remain).to_string();
            hasRemain = true;
            break;
        case 10:
            [[fallthrough]];
        case 11:
            remainBits = std::bitset<4>(remain).to_string();
            hasRemain = true;
            break;
        case 12:
            [[fallthrough]];
        case 13:
            remainBits = std::bitset<5>(remain).to_string();
            hasRemain = true;
            break;
        case 14:
            [[fallthrough]];
        case 15:
            remainBits = std::bitset<6>(remain).to_string();
            hasRemain = true;
            break;
        case 16:
            [[fallthrough]];
        case 17:
            remainBits = std::bitset<7>(remain).to_string();
            hasRemain = true;
            break;
        case 18:
            [[fallthrough]];
        case 19:
            remainBits = std::bitset<8>(remain).to_string();
            hasRemain = true;
            break;
        case 20:
            [[fallthrough]];
        case 21:
            remainBits = std::bitset<9>(remain).to_string();
            hasRemain = true;
            break;
        case 22:
            [[fallthrough]];
        case 23:
            remainBits = std::bitset<10>(remain).to_string();
            hasRemain = true;
            break;
        case 24:
            [[fallthrough]];
        case 25:
            remainBits = std::bitset<11>(remain).to_string();
            hasRemain = true;
            break;
        case 26:
            [[fallthrough]];
        case 27:
            remainBits = std::bitset<12>(remain).to_string();
            hasRemain = true;
            break;
        case 28:
            [[fallthrough]];
        case 29:
            remainBits = std::bitset<13>(remain).to_string();
            hasRemain = true;
            break;
        default:
            break;
    }
    this->destBits.append(codeBits);
    if(hasRemain)
    {
        std::reverse(remainBits.begin(), remainBits.end());
        this->destBits.append(remainBits);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::outputHLitDistModified
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::outputHLitDistModified(size_t block)
{
    std::string codeBits;
    std::string remainBits;
    for(auto index = 0; index < this->hLitDistModified[block].size(); index++)
    {
        bool hasRemain{false};
        codeBits.clear();
        remainBits.clear();
        code_t code{this->hLitDistModified[block][index].first};
        code_t remain{this->hLitDistModified[block][index].second};
        codeBits = this->lenBitCodes[block][code].second;
        switch(code)
        {
            case 16:
                remainBits = std::bitset<2>(remain).to_string();
                hasRemain = true;
                break;
            case 17:
                remainBits = std::bitset<3>(remain).to_string();
                hasRemain = true;
                break;
            case 18:
                remainBits = std::bitset<7>(remain).to_string();
                hasRemain = true;
                break;
            default:
                break;
        }
        this->destBits.append(codeBits);
        if(hasRemain)
        {
            std::reverse(remainBits.begin(), remainBits.end());
            this->destBits.append(remainBits);
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::outputLitLenCode
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::outputLitLenCode(const std::string& codeBits, code_t code, code_t remain)
{
    bool hasRemain{false};
    std::string remainBits;
    switch(code)
    {
        case 265:
            [[fallthrough]];
        case 266:
            [[fallthrough]];
        case 267:
            [[fallthrough]];
        case 268:
            remainBits = std::bitset<1>(remain).to_string();
            hasRemain = true;
            break;
        case 269:
            [[fallthrough]];
        case 270:
            [[fallthrough]];
        case 271:
            [[fallthrough]];
        case 272:
            remainBits = std::bitset<2>(remain).to_string();
            hasRemain = true;
            break;
        case 273:
            [[fallthrough]];
        case 274:
            [[fallthrough]];
        case 275:
            [[fallthrough]];
        case 276:
            remainBits = std::bitset<3>(remain).to_string();
            hasRemain = true;
            break;
        case 277:
            [[fallthrough]];
        case 278:
            [[fallthrough]];
        case 279:
            [[fallthrough]];
        case 280:
            remainBits = std::bitset<4>(remain).to_string();
            hasRemain = true;
            break;
        case 281:
            [[fallthrough]];
        case 282:
            [[fallthrough]];
        case 283:
            [[fallthrough]];
        case 284:
            remainBits = std::bitset<5>(remain).to_string();
            hasRemain = true;
            break;
        default:
            break;
    }
    this->destBits.append(codeBits);
    if(hasRemain)
    {
        std::reverse(remainBits.begin(), remainBits.end());
        this->destBits.append(remainBits);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::outputModified
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::outputModified(size_t block)
{
    std::string codeBits;
    auto iter = this->modified[block].begin();
    while(iter != this->modified[block].end())
    {
        codeBits.clear();
        code_t code{iter->first};
        code_t remain{iter->second};
        codeBits = this->litBitCodes[block][code].second;
        this->outputLitLenCode(codeBits, code, remain);
        if(code > 256)
        {
            iter++;
            codeBits.clear();
            code_t code{iter->first};
            code_t remain{iter->second};
            codeBits = this->distBitCodes[block][code].second;
            this->outputDistCode(codeBits, code, remain);
        }
        iter++;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::repairIndex
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::repairIndex(size_t oldIndex, dataMatch* match)
{
    if(oldIndex != match->getStart())
    {
        size_t newIndex{match->getStart()};
        if(this->matchList.count(newIndex) > 0)
        {
            delete this->matchList[newIndex];
        }
        this->matchList[newIndex] = match;
        this->matchList.erase(oldIndex);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::reportTime
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::reportTime(nowPoint tStart, nowPoint tEnd)
{
    auto elapsedMills{std::chrono::duration_cast<std::chrono::milliseconds>(tEnd - tStart).count()};
    double elapsedSeconds{elapsedMills / 1000.0};
    std::cout.precision(3);
    std::cout << std::endl << "Elapsed time: " << elapsedSeconds << " seconds" << std::endl;

    std::cout << std::endl << std::endl;
    this->dumpDivider();
    this->dumpDivider();
    std::cout << std::endl << std::endl;
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::sortDistCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::sortDistCodes(size_t blockIndex)
{
    this->codeBuilder.clear();
    auto countIter = this->distNumCounts[blockIndex].begin();
    while(countIter != this->distNumCounts[blockIndex].end())
    {
        dataNode newNode = dataNode();
        newNode.code = countIter->first;
        newNode.weight = countIter->second;
        this->codeBuilder.insert(std::move(newNode));
        countIter++;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::sortLenCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::sortLenCodes(size_t blockIndex)
{
    this->codeBuilder.clear();
    auto countIter = this->hclenCounts[blockIndex].begin();
    while(countIter != this->hclenCounts[blockIndex].end())
    {
        dataNode newNode = dataNode();
        newNode.code = countIter->first;
        newNode.weight = countIter->second;
        this->codeBuilder.insert(std::move(newNode));
        countIter++;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::sortLitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::sortLitCodes(size_t blockIndex)
{
    this->codeBuilder.clear();
    auto countIter = this->litNumCounts[blockIndex].begin();
    while(countIter != this->litNumCounts[blockIndex].end())
    {
        dataNode newNode = dataNode();
        newNode.code = countIter->first;
        newNode.weight = countIter->second;
        this->codeBuilder.insert(std::move(newNode));
        countIter++;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::trimTwoFer
///////////////////////////////////////////////////////////////////////////////

matchListIter workerDeflate::trimTwoFer(matchListIter oneIter)
{
    matchListIter returnIter{oneIter};
    size_t numCulled{0};
    dataMatch *one{oneIter->second};
    size_t oneStart = one->getStart();
    size_t oneLength = one->getLength();
    size_t oneFlex = oneLength - 3;
    size_t oneEnd = oneStart + (oneLength - 1);

    auto twoIter{std::next(oneIter,1)};
    dataMatch *two{twoIter->second};
    size_t twoStart = two->getStart();
    size_t twoLength = two->getLength();
    size_t twoFlex = twoLength - 3;
    size_t twoEnd = twoStart + (twoLength - 1);
    
    if(twoStart <= oneEnd)
    {
        size_t front{twoStart - oneStart};
        size_t back{twoEnd - oneEnd};
        size_t overlap{1 + (oneEnd - twoStart)};
        size_t gain{front + back};

        if((oneFlex + twoFlex) >= overlap && gain > 2)
        {
            if(oneFlex >= twoFlex)
            {
                if(oneFlex <= overlap)
                {
                    one->trimBack(oneFlex);
                    overlap -= oneFlex;
                    two->trimFront(overlap);
                    this->repairIndex(twoStart, two);
                }
                else
                {
                    one->trimBack(overlap);
                }
            }
            else
            {
                if(twoFlex <= overlap)
                {
                    two->trimFront(twoFlex);
                    this->repairIndex(twoStart, two);
                    overlap -= twoFlex;
                    one->trimBack(overlap);
                }
                else
                {
                    two->trimFront(overlap);
                    this->repairIndex(twoStart, two);
                }
            }
        }
        else
        {
            if(front > back)
            {
                delete two;
                this->matchList.erase(twoIter);
            }
            else if(front == back)
            {
                if(one->getDistance() <= two->getDistance())
                {
                    delete two;
                    this->matchList.erase(twoIter);
                }
                else
                {
                    delete one;
                    returnIter = this->matchList.erase(oneIter);
                }                    
            }
            else
            {
                delete one;
                returnIter = this->matchList.erase(oneIter);
            }
        }
    }    
    return(returnIter);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::updateHLitDistModified
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::updateHLitDistModified(size_t block, const code_t& cur, code_t& last, code_t& repeat)
{
    if(last == 0)
    {
        repeat += 1;
    }
    if(last == 0 && repeat > 10)
    {
        while(repeat > 10)
        {
            if(repeat > 138)
            {
                this->hLitDistModified[block].push_back({18, 127});
                this->hclenCounts[block][18]++;
                repeat -= 138;
            }
            else
            {
                this->hLitDistModified[block].push_back({18, (repeat - 11)});
                this->hclenCounts[block][18]++;
                repeat = 0;
            }                 
        }
        if(repeat > 0)
        {
            this->hLitDistModified[block].push_back({17, (repeat - 3)});
            this->hclenCounts[block][17]++;
            repeat = 0;
        }
    }
    else if(last == 0 && repeat >= 3)
    {
        this->hLitDistModified[block].push_back({17, (repeat - 3)});
        this->hclenCounts[block][17]++;
        repeat = 0;
    }
    else if(last == 0)
    {
        while(repeat > 0)
        {
            this->hLitDistModified[block].push_back({last, 0});
            this->hclenCounts[block][last]++;
            repeat--;
        }
    }
    else if(repeat >= 3)
    {
        while(repeat > 0)
        {
            if(repeat > 6)
            {
                this->hLitDistModified[block].push_back({16, 3});
                this->hclenCounts[block][16]++;
                repeat -= 6;
            }
            else
            {
                this->hLitDistModified[block].push_back({16, (repeat - 3)});
                this->hclenCounts[block][16]++;
                repeat = 0;
            }                 
        }            
    }
    else if(repeat > 0)
    {
        while(repeat > 0)
        {
            this->hLitDistModified[block].push_back({last, 0});
            this->hclenCounts[block][last]++;
            repeat--;
        }
    }               
    if(cur != 0 && cur != last)
    {
        this->hLitDistModified[block].push_back({cur, 0});
        this->hclenCounts[block][cur]++;
    }
    last = cur;
    repeat = 0;
}

///////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerDeflate::~workerDeflate()
{

}