//
// erasmus_director.cpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Author: Joseph Adomatis
// Copyright (c) 2020 Joseph R Adomatis (joseph dot adomatis at gmail dot com)
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../headers/erasmus_namespace.hpp"
#include "../headers/erasmus_director.hpp"
#include "../headers/worker_base64.hpp"
#include "../headers/worker_case.hpp"
#include "../headers/worker_extract.hpp"
#include "../headers/worker_numeric.hpp"
#include "../headers/worker_replace.hpp"
#include "../headers/worker_url.hpp"

using namespace erasmus;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global variable definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// global function definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// erasmus:: member definitions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////

director::director()
{
    this->base64Worker = nullptr;
    this->caseWorker = nullptr;
    this->extractWorker = nullptr;
    this->numericWorker = nullptr;
    this->replaceWorker = nullptr;
    this->urlWorker = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::base64Decode
///////////////////////////////////////////////////////////////////////////////

bool director::base64Decode(const std::string& original, std::string& revised)
{
    if(this->base64Worker == nullptr)
    {
        this->base64Worker = new workerBase64();
    }
    return(this->base64Worker->base64Decode(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::base64Encode
///////////////////////////////////////////////////////////////////////////////

bool director::base64Encode(const std::string& original, std::string& revised)
{
    if(this->base64Worker == nullptr)
    {
        this->base64Worker = new workerBase64();
    }
    return(this->base64Worker->base64Encode(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::charToHex
///////////////////////////////////////////////////////////////////////////////

bool director::charToHex(char original, std::string& revised)
{
    if(this->numericWorker == nullptr)
    {
        this->numericWorker = new workerNumeric();
    }
    return(this->numericWorker->charToHex(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::escapeReplace
///////////////////////////////////////////////////////////////////////////////

bool director::escapeReplace(const std::string& original, std::string& revised)
{
    if(this->replaceWorker == nullptr)
    {
        this->replaceWorker = new workerReplace();
    }
    return(this->replaceWorker->escapeReplace(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::isHex
///////////////////////////////////////////////////////////////////////////////

bool director::isHex(char c)
{
    if(this->numericWorker == nullptr)
    {
        this->numericWorker = new workerNumeric();
    }
    return(this->numericWorker->isHex(c));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::mimeEncode
///////////////////////////////////////////////////////////////////////////////

bool director::mimeEncode(const std::string& original, std::string& revised)
{
    if(this->base64Worker == nullptr)
    {
        this->base64Worker = new workerBase64();
    }
    return(this->base64Worker->mimeEncode(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::quotedExtract
///////////////////////////////////////////////////////////////////////////////

bool director::quotedExtract(const std::string& original, std::string& revised, bool escape)
{
    if(this->extractWorker == nullptr)
    {
        this->extractWorker = new workerExtract(this);
    }
    return(this->extractWorker->quotedExtract(original, revised, escape));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::urlDecode
///////////////////////////////////////////////////////////////////////////////

bool director::urlDecode(const std::string& original, std::string& revised)
{
    if(this->urlWorker == nullptr)
    {
        this->urlWorker = new workerURL(this);
    }
    return(this->urlWorker->urlDecode(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::urlEncode
///////////////////////////////////////////////////////////////////////////////

bool director::urlEncode(const std::string& original, std::string& revised)
{
    if(this->urlWorker == nullptr)
    {
        this->urlWorker = new workerURL(this);
    }
    return(this->urlWorker->urlEncode(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::tokenReplace
///////////////////////////////////////////////////////////////////////////////

bool director::tokenReplace(std::string& source, const std::string& oldToken, const std::string& newToken)
{
    if(this->replaceWorker == nullptr)
    {
        this->replaceWorker = new workerReplace();
    }
    return(this->replaceWorker->tokenReplace(source, oldToken, newToken));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::toLower
///////////////////////////////////////////////////////////////////////////////

bool director::toLower(const std::string& original, std::string& revised)
{
    if(this->caseWorker == nullptr)
    {
        this->caseWorker = new workerCase();
    }
    return(this->caseWorker->toLower(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::toUpper
///////////////////////////////////////////////////////////////////////////////

bool director::toUpper(const std::string& original, std::string& revised)
{
    if(this->caseWorker == nullptr)
    {
        this->caseWorker = new workerCase();
    }
    return(this->caseWorker->toUpper(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// erasmus::director::uintToHex
///////////////////////////////////////////////////////////////////////////////

bool director::uintToHex(size_t original, std::string& revised)
{
    if(this->numericWorker == nullptr)
    {
        this->numericWorker = new workerNumeric();
    }
    return(this->numericWorker->uintToHex(original, revised));
}

///////////////////////////////////////////////////////////////////////////////
// Deconstructor
///////////////////////////////////////////////////////////////////////////////

director::~director()
{
    if(this->base64Worker != nullptr)
    {
        delete this->base64Worker;
        this->base64Worker = nullptr;
    }
    if(this->caseWorker != nullptr)
    {
        delete this->caseWorker;
        this->caseWorker = nullptr;
    }
    if(this->extractWorker != nullptr)
    {
        delete this->extractWorker;
        this->extractWorker = nullptr;
    }
    if(this->numericWorker != nullptr)
    {
        delete this->numericWorker;
        this->numericWorker = nullptr;
    }
    if(this->replaceWorker != nullptr)
    {
        delete this->replaceWorker;
        this->replaceWorker = nullptr;
    }
    if(this->urlWorker != nullptr)
    {
        delete this->urlWorker;
        this->urlWorker = nullptr;
    }
}