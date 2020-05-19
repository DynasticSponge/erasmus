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

typedef unsigned short code_t;
typedef std::map<size_t, erasmus::dataMatch*>::iterator matchListIter;
typedef std::chrono::time_point<std::chrono::steady_clock> nowPoint;
typedef std::pair<code_t, code_t> codePair;

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
    void dumpTree();
    void dumpTreeLineBits(std::string &, code_t, std::string);
    void dumpTreeLineLead(std::string&, size_t, size_t, code_t);
    void generateDistBitCodeLens(size_t, const dataNode&, code_t);
    void generateDistBitCodes(size_t);
    void generateHCLen(size_t);
    void generateHCLenModified(size_t);
    void generateHDist(size_t);
    void generateHLitDistModified(size_t);
    void generateHLit(size_t);
    void generateLenBitCodeLens(size_t, const dataNode&, code_t);
    void generateLenBitCodes(size_t);
    void generateLitBitCodeLens(size_t, const dataNode&, code_t);
    void generateLitBitCodes(size_t);
    void generateModified();
    void generateTree();
    std::string getBitCodeString(code_t, code_t);
    codePair getDistCode(size_t);
    codePair getLitCode(size_t);
    void growMatch(size_t);
    void initDistCodes();
    void initDistBitCodes(size_t);
    void initLenBitCodes(size_t);
    void initLitCodes();
    void initLitBitCodes(size_t);
    std::string outputDestination();
    void outputDistCode(const std::string&, code_t, code_t);
    void outputHLitDistModified(size_t);
    void outputLitLenCode(const std::string&, code_t, code_t);
    void outputModified(size_t);  
    void repairIndex(size_t, erasmus::dataMatch*);
    void reportTime(nowPoint, nowPoint);
    void sortDistCodes(size_t);
    void sortLenCodes(size_t);
    void sortLitCodes(size_t);
    matchListIter trimTwoFer(matchListIter);
    void updateHLitDistModified(size_t, const code_t&, code_t&, code_t&);
    ///////////////////////////////////////////////////////////////////////////////
    // Private Properties
    ///////////////////////////////////////////////////////////////////////////////
    std::string source;
    std::string destBits;
    std::string_view original;
    std::map<size_t, erasmus::dataMatch*> matchList;
    std::map<code_t, code_t> distNumCodes;
    std::map<code_t, code_t> litNumCodes;
    std::set<dataNode> codeBuilder;
    std::vector<code_t> hlit;
    std::vector<code_t> hdist;
    std::vector<code_t> hclen;
    std::vector<std::vector<codePair>> modified;
    std::vector<std::vector<codePair>> hLitDistModified;
    std::vector<std::vector<codePair>> hclenModified;
    std::vector<std::map<code_t, code_t>> hclenCounts;
    std::vector<std::map<code_t, code_t>> distNumCounts;
    std::vector<std::map<code_t, code_t>> litNumCounts;
    std::vector<std::map<code_t, std::pair<code_t, std::string>>> distBitCodes;
    std::vector<std::map<code_t, std::pair<code_t, std::string>>> lenBitCodes;
    std::vector<std::map<code_t, std::pair<code_t, std::string>>> litBitCodes;
};

#endif