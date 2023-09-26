#pragma once
#include <fstream>
#include <direct.h>
#include "Nlohmann/json.hpp"
#include "WebSocketClient.h"
#include "Logger.h"
#include "daemon.h"

cyanray::WebSocketClient ws;
Logger logger("Wheat");
nlohmann::json Config;


bool loadConfig() {
    // load config.json
    std::ifstream file;
    file.open("Config.json", std::ios::in);

    std::istreambuf_iterator<char> beg(file), end;
    string configString = std::string(beg, end);
    file.seekg(0, std::ios::end);//移动的文件尾部
    file.close();
    try {
        Config = nlohmann::json::parse(configString.c_str(), nullptr, true);
    }
    catch (const std::exception& ex) {
        logger.error(ex.what());
        return false;
    }
    return true;
}
