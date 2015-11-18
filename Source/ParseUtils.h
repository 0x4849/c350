#pragma once

#include "Common.h"

namespace UAlbertaBot
{
namespace ParseUtils
{
    void ParseConfigFile(const std::string & filename); // parses Config file, in particular it sends strategies to StrategyManager
    void ParseTextCommand(const std::string & commandLine);
    BWAPI::Race GetRace(const std::string & raceName);

    int GetIntFromString(const std::string & str);
    bool GetBoolFromString(const std::string & str);







}
}