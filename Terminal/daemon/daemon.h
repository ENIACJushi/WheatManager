#pragma once
#include <Windows.h>
#include "WebAPI.h"
#include "Config.h"
#include "Tools.h"
struct Server {
    Server(string n, string p, bool e) {
        enable = e;
        name = n;
        path = p;
        life = 2;
    }
    bool enable;
    string name;
    string path;
    int life;
};

class Daemon
{    
    vector<Server> serverList;
    int timeout;
    int timeLeft;
    int startTime;
public:
    Daemon() {
        timeout = 1;
        timeLeft = 1;
        startTime = 60;
    }
    void initiate(nlohmann::json serverListJson, int timeout_, int startTime_) {
        for (int i = 0; i < serverListJson.size(); i++) {
            serverList.push_back(Server(serverListJson[i]["name"], serverListJson[i]["path"], serverListJson[i]["enable"]));
        }
        timeout = timeout_;
        timeLeft = timeout;
        startTime = startTime_;
    }
    void tick() {
        if (timeLeft > 0) {
            timeLeft--;
        }
        else {
            timeLeft = timeout;
            for (int i = 0; i < serverList.size(); i++) {
                if (serverList[i].enable) {
                    // ping
                    WebAPI::pingServer(serverList[i].name, "daemon", serverList[i].name);
                    // wither
                    if (serverList[i].life > 0) {
                        serverList[i].life--;
                    }
                    // wakeUp
                    if (serverList[i].life <= 0) {
                        logger.info("Wake up server " + serverList[i].name + " at " + serverList[i].path);
                        wakeUp(serverList[i].path);
                        serverList[i].life = startTime;
                    }
                }
            }
        }
    }

    void cure(string name) {
        for (int i = 0; i < serverList.size(); i++) {
            if (serverList[i].name == name) {
                serverList[i].life = 2;
            }
        }
    }
    void wakeUp(string path) {
        startBDS(path);
    }
};
