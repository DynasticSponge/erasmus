//
// worker_deflate.cpp
// ~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
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
    this->initLenCodes();
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
        size_t saveIndex{matchIter->first};
        
        size_t maxNextStart{matchIter->first + (matchIter->second->getLength() + 2)};
        size_t nextStart{matchIter->first};
        size_t maxEnd{matchIter->first + (matchIter->second->getLength() - 1)};
        
        auto searchIter{std::next(matchIter, 1)};
        if(searchIter != this->matchList.end())
        {
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
    this->buildMatches();
    this->cullMatches();
    this->generateModified();
    for(auto i = 0; i < this->modified.size(); i++)
    {
        std::string blankStr;
        this->initDistBitCodes();
        this->initLenBitCodes(); 
        this->sortLitLenCodes(i);
        this->generateTree();
        this->generateLenBitCodes(*(this->codeBuilder.begin()), blankStr);
        this->sortDistCodes(i);
        this->generateTree();
        this->generateDistBitCodes(*(this->codeBuilder.begin()), blankStr);
    }
    nowPoint tEnd{std::chrono::steady_clock::now()};
    
    this->dumpTree(0);
    this->reportTime(tStart, tEnd);
    //this->dumpMatchList();
    
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

void workerDeflate::dumpTree(size_t block)
{
    std::cout.setf(std::ios::left, std::ios::adjustfield);
    std::string tempStr;

    std::cout << "Literal/Length Bit Codes" << std::endl << std::endl;
    
    for(unsigned short i = 0; i < 286; i++)
    {
        if(this->lenNumCounts[block][i] == 0)
        {
            continue;
        }
        std::cout.width(16);
        tempStr.clear();
        tempStr.append("Code: ");
        tempStr.append(std::to_string(i));
        std::cout << tempStr;

        std::cout.width(16);
        tempStr.clear();
        tempStr.append("Count: ");
        tempStr.append(std::to_string(this->lenNumCounts[block][i]));
        std::cout << tempStr;

        std::cout.width(32);
        tempStr.clear();
        tempStr.append("BitCode: ");
        tempStr.append(this->lenBitCodes[i]);
        std::cout << tempStr;

        std::cout.width(32);
        tempStr.clear();
        tempStr.append("Bit Length: ");
        tempStr.append(std::to_string(this->lenBitCodes[i].size()));
        std::cout << tempStr;

        std::cout << std::endl;
    }
    std::cout << std::endl;
    std::cout << "Distance Bit Codes" << std::endl << std::endl;
    for(unsigned short i = 0; i < 30; i++)
    {
        if(this->distNumCounts[block][i] == 0)
        {
            continue;
        }
        std::cout.width(16);
        tempStr.clear();
        tempStr.append("Code: ");
        tempStr.append(std::to_string(i));
        std::cout << tempStr;

        std::cout.width(16);
        tempStr.clear();
        tempStr.append("Count: ");
        tempStr.append(std::to_string(this->distNumCounts[block][i]));
        std::cout << tempStr;

        std::cout.width(32);
        tempStr.clear();
        tempStr.append("BitCode: ");
        tempStr.append(this->distBitCodes[i]);
        std::cout << tempStr;

        std::cout.width(32);
        tempStr.clear();
        tempStr.append("Bit Length: ");
        tempStr.append(std::to_string(this->distBitCodes[i].size()));
        std::cout << tempStr;

        std::cout << std::endl;
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
    std::map<unsigned short, size_t> litLenCounts;
    for(short i=0; i < 285; i++)
    {
        litLenCounts[i] = 0;
    }
    std::map<unsigned short, size_t> distCounts;
    for(short i=0; i < 30; i++)
    {
        distCounts[i] = 0;
    }
    while(sourceIndex < this->source.size())
    {
        codePair newCode;
        if(this->matchList.count(sourceIndex) > 0)
        {
            size_t length = this->matchList[sourceIndex]->getLength();
            newCode = this->getLenCode(length);
            block.push_back(newCode);
            litLenCounts[newCode.first] += 1;
            newCode = this->getDistCode(this->matchList[sourceIndex]->getDistance());
            block.push_back(newCode);
            distCounts[newCode.first] += 1;
            sourceIndex += length;
            blockIndex += length;
            delete this->matchList[length];
            this->matchList.erase(length);
        }
        else
        {
            newCode.first = static_cast<unsigned short>(static_cast<unsigned char>(this->source[sourceIndex]));
            newCode.second = 0;
            block.push_back(newCode);
            litLenCounts[newCode.first] += 1;
            sourceIndex++;
        }
        if(blockIndex == 65534)
        {
            newCode.first = 256;
            newCode.second = 0;
            block.push_back(newCode);
            litLenCounts[newCode.first] += 1;
            this->modified.push_back(std::move(block));
            this->lenNumCounts.push_back(std::move(litLenCounts));
            this->distNumCounts.push_back(std::move(distCounts));
            distCounts.clear();
            for(short i=0; i < 30; i++)
            {
                distCounts[i] = 0;
            }
            litLenCounts.clear();
            for(short i=0; i < 285; i++)
            {
                litLenCounts[i] = 0;
            }
            block.clear();            
            blockIndex = 0;
        }
        else
        {
            blockIndex++;
        }                
    }
    this->modified.push_back(std::move(block));
    this->lenNumCounts.push_back(std::move(litLenCounts));
    this->distNumCounts.push_back(std::move(distCounts));
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateDistBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateDistBitCodes(const dataNode& curNode, const std::string& parent)
{
    if(curNode.hasKids > 0)
    {   
        std::string newParent{parent};
        newParent.append("0");    
        this->generateDistBitCodes(curNode.kids[0], newParent);
        if(curNode.hasKids > 1)
        {
            newParent.pop_back();
            newParent.append("1");
            this->generateDistBitCodes(curNode.kids[1], newParent);
        }        
    }
    else
    {
        this->distBitCodes[curNode.code] = parent;
    }
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::generateLenBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::generateLenBitCodes(const dataNode& curNode, const std::string& parent)
{
    if(curNode.hasKids > 0)
    {   
        std::string newParent{parent};
        newParent.append("0");    
        this->generateLenBitCodes(curNode.kids[0], newParent);
        if(curNode.hasKids > 1)
        {
            newParent.pop_back();
            newParent.append("1");
            this->generateLenBitCodes(curNode.kids[1], newParent);
        }
    }
    else
    {
        this->lenBitCodes[curNode.code] = parent;
    }
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
            returnCode.first = static_cast<unsigned short>(iter->second);
            returnCode.second = static_cast<unsigned short>(distance - prevMax);
            codeFound = true;
        }        
    }
    return(returnCode);
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::getLenCode
///////////////////////////////////////////////////////////////////////////////

codePair workerDeflate::getLenCode(size_t length)
{
    codePair returnCode;
    bool codeFound{false};
    size_t prevMax{0};
    auto iter{this->lenNumCodes.begin()};
    while(!codeFound)
    {
        if(length > iter->first)
        {
            prevMax = iter->first + 1;
            iter++;
        }
        else
        {
            returnCode.first = static_cast<unsigned short>(iter->second);
            returnCode.second = static_cast<unsigned short>(length - prevMax);
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
    this->initDistBitCodes();
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initDistBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initDistBitCodes()
{
    this->distBitCodes.clear();
    for(unsigned short i = 0; i < 30; i++)
    {
        std::string newStr;
        this->distBitCodes[i] = std::move(newStr);
    }
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initLenCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initLenCodes()
{
    this->lenNumCodes.insert({3,257});
    this->lenNumCodes.insert({4,258});
    this->lenNumCodes.insert({5,259});
    this->lenNumCodes.insert({6,260});
    this->lenNumCodes.insert({7,261});
    this->lenNumCodes.insert({8,262});
    this->lenNumCodes.insert({9,263});
    this->lenNumCodes.insert({10,264});
    this->lenNumCodes.insert({12,265});
    this->lenNumCodes.insert({14,266});
    this->lenNumCodes.insert({16,267});
    this->lenNumCodes.insert({18,268});
    this->lenNumCodes.insert({22,269});
    this->lenNumCodes.insert({26,270});
    this->lenNumCodes.insert({30,271});
    this->lenNumCodes.insert({34,272});
    this->lenNumCodes.insert({42,273});
    this->lenNumCodes.insert({50,274});
    this->lenNumCodes.insert({58,275});
    this->lenNumCodes.insert({66,276});
    this->lenNumCodes.insert({82,277});
    this->lenNumCodes.insert({98,278});
    this->lenNumCodes.insert({114,279});
    this->lenNumCodes.insert({130,280});
    this->lenNumCodes.insert({162,281});
    this->lenNumCodes.insert({194,282});
    this->lenNumCodes.insert({226,283});
    this->lenNumCodes.insert({257,284});
    this->lenNumCodes.insert({258,285});
    this->initLenBitCodes();
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::initLenBitCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::initLenBitCodes()
{
    this->lenBitCodes.clear();
    for(unsigned short i = 0; i < 286; i++)
    {
        std::string newStr;
        this->lenBitCodes[i] = std::move(newStr);
    }
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
        dataNode newNode;
        newNode.code = countIter->first;
        newNode.weight = countIter->second;
        this->codeBuilder.insert(newNode);
        countIter++;
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate::sortLitLenCodes
///////////////////////////////////////////////////////////////////////////////

void workerDeflate::sortLitLenCodes(size_t blockIndex)
{
    this->codeBuilder.clear();
    auto countIter = this->lenNumCounts[blockIndex].begin();
    while(countIter != this->lenNumCounts[blockIndex].end())
    {
        dataNode newNode;
        newNode.code = countIter->first;
        newNode.weight = countIter->second;
        this->codeBuilder.insert(newNode);
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
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

workerDeflate::~workerDeflate()
{

}