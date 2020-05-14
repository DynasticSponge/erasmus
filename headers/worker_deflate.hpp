//
// worker_deflate.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef WORKER_DEFLATE_HPP
#define WORKER_DEFLATE_HPP

#include <map>
#include <queue>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "erasmus_namespace.hpp"
#include "data_match.hpp"
#include "data_node.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Related Type Definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<size_t, erasmus::dataMatch*>::iterator matchListIter;
typedef std::chrono::time_point<std::chrono::steady_clock> nowPoint;
typedef std::pair<unsigned short, unsigned short> codePair;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus::workerDeflate Definition
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class erasmus::workerDeflate
{
public:
    explicit workerDeflate(std::string);
    std::string deflate();
    std::string inflate();
    ~workerDeflate();
protected:
private:
    ///////////////////////////////////////////////////////////////////////////////
    // Private Functions
    ///////////////////////////////////////////////////////////////////////////////
    void buildMatches();
    void cullMatches();
    void dumpDivider();
    void dumpHeader(size_t, size_t);
    size_t dumpMatch(size_t, size_t, size_t);
    void dumpMatchList();
    void dumpTree(size_t);
    void dumpTreeLineLead(std::string&, size_t, size_t);
    codePair getDistCode(size_t);
    codePair getLenCode(size_t);
    void generateModified();
    void generateTree();
    void generateDistBitCodes(const dataNode&, const std::string&);
    void generateLenBitCodes(const dataNode&, const std::string&);
    void growMatch(size_t);
    void initDistCodes();
    void initDistBitCodes();
    void initLenCodes();
    void initLenBitCodes();    
    void repairIndex(size_t, erasmus::dataMatch*);
    void reportTime(nowPoint, nowPoint);
    void sortDistCodes(size_t);
    void sortLitLenCodes(size_t);
    matchListIter trimTwoFer(matchListIter);
    ///////////////////////////////////////////////////////////////////////////////
    // Private Properties
    ///////////////////////////////////////////////////////////////////////////////
    std::string source;
    std::string destination;
    std::string_view original;
    std::map<size_t, erasmus::dataMatch*> matchList;
    std::map<unsigned short, unsigned short> distNumCodes;
    std::map<unsigned short, std::string> distBitCodes;
    std::map<unsigned short, std::string> lenBitCodes;
    std::map<unsigned short, unsigned short> lenNumCodes;
    std::set<dataNode> codeBuilder;
    std::vector<std::vector<codePair>> modified;
    std::vector<std::map<unsigned short, size_t>> distNumCounts;
    std::vector<std::map<unsigned short, size_t>> lenNumCounts;
};

#endif