
// ******* Configurations ******* //
#pragma once

#include <LLAPI.h>
#include <LoggerAPI.h>
#include <EventAPI.h>

#include <MC/Actor.hpp>
#include <MC/ActorDamageSource.hpp>
#include <MC/CompoundTag.hpp>
#include <MC/ItemStack.hpp>
#include <MC/Level.hpp>
#include <MC/ListTag.hpp>
#include <MC/NbtIo.hpp>
#include <MC/NBTSaver.hpp>
#include <MC/NBTLoader.hpp>
#include <MC/Player.hpp>
#include <MC/Scoreboard.hpp>
#include <MC/CompoundTag.hpp>
#include <LLAPI.h>
#include <ScheduleAPI.h>
#include <Utils/NetworkHelper.h>

#include <fstream>
#include <direct.h>
#include <string>
#include <Nlohmann/json.hpp>
#include <SimpleIni/SimpleIni.h>

#include "WebSocketClient.h"
#include "Language.h"

// debug
const int setBagVersion = 2;
// other plugins
typedef long long money_t;
typedef money_t (*LLMoneyGet_T)(xuid_t);
typedef bool (*LLMoneySet_T)(xuid_t, money_t);
struct dynamicSymbolsMap_type
{
    LLMoneyGet_T LLMoneyGet = nullptr;
    LLMoneySet_T LLMoneySet = nullptr;
} dynamicSymbolsMap;
// logger
Logger logger("Wheat");

// language
string languageName;
// base
string hostURL = "localhost";
string serverName = "world";
string serverIP = "localhost";
// Web socket
cyanray::WebSocketClient ws;
string ws_hostURL = "ws://localhost/ws/asset";
string ws_key = "key";
int    ws_timeOut = 30;
// Synchronize
int    syn_autoSaveTime = 5;
bool   syn_bag = true;
bool   syn_enderChest = true;
bool   syn_attributes = true;
bool   syn_level = true;
bool   syn_scores = true;
bool   syn_tags = true;
bool   syn_message = true;
bool   syn_money = true;
string syn_moneyType = "score";
string syn_moneyName = "money";

void loadConfig() {
    /* 
       Floder
       "plugins/Wheat"
       "plugins/Wheat/text"
    */
    if (_access("plugins/Wheat", 0) != 0) {
        if (!_mkdir("plugins/Wheat")) logger.fatal("Dir \"plugins/Wheat\" make failed.");
    }
    if (_access("plugins/Wheat/texts", 0) != 0) {
        if (!_mkdir("plugins/Wheat/texts")) logger.fatal("Dir \"plugins/Wheat/texts\" make failed.");
    }
    /*
        Other plugins
         LLMoney
    */
    LL::Plugin* llmoney = LL::getPlugin("LLMoney");
    if (!llmoney) logger.error("init.llMoney.noFound");
    else {
        HMODULE h = llmoney->handler;
        dynamicSymbolsMap.LLMoneyGet = (LLMoneyGet_T)GetProcAddress(h, "LLMoneyGet");
        if (!dynamicSymbolsMap.LLMoneyGet) logger.error("Fail to load API money.getMoney!");
        dynamicSymbolsMap.LLMoneySet = (LLMoneySet_T)GetProcAddress(h, "LLMoneySet");
        if (!dynamicSymbolsMap.LLMoneySet) logger.error("Fail to load API money.setMoney!");
    }

    // Load "Config.ini"
    CSimpleIniA ini;
    SI_Error rc = ini.LoadFile("plugins/Wheat/Config.ini");
    // handle error, create default config file
    if (rc < 0) {
        logger.warn("Config.ini doesn't exist, create default file.");
        std::ofstream configFile;
        configFile.open("plugins/Wheat/Config.ini", std::ios::out | std::ios::_Noreplace);
        // configFile << "";
        configFile.close();
        ini.SetValue("Base", "hostURL", "localhost");
        ini.SetValue("Base", "serverName", "world");
        ini.SetValue("Base", "serverIP", "localhost");
        ini.SetValue("Base", "language"  , "zh_CN");
        ini.SetValue("Web Socket", "hostURL", "ws://localhost/ws/asset");
        ini.SetValue("Web Socket", "key", "key");
        ini.SetValue("Web Socket", "timeOut", "15");
        ini.SetValue("Synchronization", "autoSaveTime", "5");
        ini.SetValue("Synchronization", "bag", "true");
        ini.SetValue("Synchronization", "enderChest", "true");
        ini.SetValue("Synchronization", "attributes", "true");
        ini.SetValue("Synchronization", "level", "true");
        ini.SetValue("Synchronization", "scores", "true");
        ini.SetValue("Synchronization", "tags", "true");
        ini.SetValue("Synchronization", "message", "true");
        ini.SetValue("Synchronization", "money", "true");
        ini.SetValue("Synchronization", "moneyType", "score");
        ini.SetValue("Synchronization", "moneyName", "money");
        rc = ini.SaveFile("plugins/Wheat/Config.ini");
        if (rc < 0) logger.error("Failed to create config file.");
        else        logger.info("Create config file successfully");
    }
    else {
        hostURL        = ini.GetValue("Base", "hostURL"   , "localhost");
        serverName     = ini.GetValue("Base", "serverName", "world");
        serverIP       = ini.GetValue("Base", "serverIP"  , "localhost");
        languageName   = ini.GetValue("Base", "language"  , "zh_CN");
        ws_hostURL     = ini.GetValue("Web Socket", "hostURL", "ws://localhost/ws/asset");
        ws_key         = ini.GetValue("Web Socket", "key", "key");
        ws_timeOut     = atoi(ini.GetValue("Web Socket", "timeOut", "15"));
        syn_autoSaveTime =  atoi(ini.GetValue("Synchronization", "autoSaveTime", "5"));
        syn_bag        = (string)ini.GetValue("Synchronization", "bag"           , "true") == "true";
        syn_enderChest = (string)ini.GetValue("Synchronization", "enderChest"    , "true") == "true";
        syn_attributes = (string)ini.GetValue("Synchronization", "attributes"    , "true") == "true";
        syn_level      = (string)ini.GetValue("Synchronization", "level"         , "true") == "true";
        syn_scores     = (string)ini.GetValue("Synchronization", "scores"        , "true") == "true";
        syn_tags       = (string)ini.GetValue("Synchronization", "tags"          , "true") == "true";
        syn_message    = (string)ini.GetValue("Synchronization", "message"       , "true") == "true";
        syn_money      = (string)ini.GetValue("Synchronization", "money"         , "true") == "true";
        syn_moneyType  = ini.GetValue("Synchronization", "moneyType", "score");
        syn_moneyName  = ini.GetValue("Synchronization", "moneyName", "money");
    }

    // Load language file
    std::ifstream languageFile;
    languageFile.open("plugins/Wheat/texts/" + languageName + ".lang");
    if (!lang.load(&languageFile)) logger.warn("languageFile load failed.");
    languageFile.close();
}

string config_toString() {
    return "Loading...\n\n   =====================================\n\t[Base]\n\t  hostURL:\t" + hostURL + "\n"
        + "\t  serverName:\t"   + serverName                          + "\n"
        + "\t  serverIP:\t"     + serverIP                            + "\n"
        + "\t  language:\t"     + languageName                        + "\n"
        + "\t[Web Socket]\n"
        + "\t  host URL:\t"     + ws_hostURL                          + "\n"
        + "\t  key:\t\t"        + ws_key                              + "\n"
        + "\t  timeOut:\t"      + std::to_string(ws_timeOut)          + "\n"
        + "\t[Synchronization]\n"
        + "\t  autoSaveTime:\t" + std::to_string(syn_autoSaveTime)    + "\n"
        + "\t  bag:\t\t"        + (syn_bag        ? "true" : "false") + "\n"
        + "\t  enderChest:\t"   + (syn_enderChest ? "true" : "false") + "\n"
        + "\t  attributes:\t"   + (syn_attributes ? "true" : "false") + "\n"
        + "\t  level:\t"        + (syn_level      ? "true" : "false") + "\n"
        + "\t  scores:\t"       + (syn_scores     ? "true" : "false") + "\n"
        + "\t  tags:\t\t"       + (syn_tags       ? "true" : "false") + "\n"
        + "\t  message:\t"      + (syn_message    ? "true" : "false") + "\n"
        + "\t  money:\t"        + (syn_money      ? "true" : "false") + "\n"
        + "\t  money type:\t"   + syn_moneyType                       + "\n"
        + "\t  money name:\t"   + syn_moneyName                       + "\n"
        + "   =====================================\n";
}
