#include <iostream>
#include <Windows.h>
#include <thread>
#include "Nlohmann/json.hpp"

#include "daemon.h"
#include "WebAPI.h"
#include "Config.h"
#include "Tools.h"

Daemon daemon;
void thread_tick() {
    while (true) {
        Sleep(1000);
        if (ws.GetStatus() == cyanray::WebSocketClient::Status::Open) {
            daemon.tick();
        }
    }
}

int main()
{
    system("chcp 65001");
    logger.info("Loading config..");
    if (!loadConfig()) {
        logger.error("Failed to load config.");
        return 0;
    }
    // load daemon
    daemon.initiate(Config["daemon"], Config["daemon_settings"]["timeout"], Config["daemon_settings"]["start_time"]);

    logger.info("Connet websocket server..");
    while (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
        Sleep(1000);
        try {
            ws.Connect(Config["web_socket"]["host_url"]);
        }
        catch (const std::exception& ex) {
            logger.error("Can not connet websocket server." + string(ex.what()));
        }
    }
    ws.OnTextReceived([](cyanray::WebSocketClient& client, string msg) {
        // logger.info(msg);
        // logger.info("[websocket] received:" + msg);
        if (msg.empty()) {
            return;
        }
        else if (msg == "success") {
            logger.info("Connection established.");
            WebAPI::identityAuthentication("onStart");
        }
        else if (!nlohmann::json::accept(msg)) {
            logger.info("[websocket] Invalid message received: " + msg);
        }
        else {
            try {
                const string m = msg;
                nlohmann::json message = nlohmann::json::parse(m.c_str(), nullptr, true);
                ///// Reply /////
                if (message["type"] == "identityAuthentication") {
                    if (message["result"] == "success")
                        logger.info("Authentication passed.");
                    else
                        logger.info("Authentication not passed.");
                }
                else if (message["type"] == "broadcastMessage") {
                    nlohmann::json broadcastMessage = message["message"];
                    if (broadcastMessage["type"] == "player_join") {
                        try {
                            logger.info(string(broadcastMessage["player"]) + " joined the game.");
                        }
                        catch (const std::exception& ex) {
                            logger.error("Error in websocket receive message~player_join." + string(ex.what()));
                        }
                    }
                    else if (broadcastMessage["type"] == "player_left") {
                        try {
                            logger.info(string(broadcastMessage["player"]) + " left the game.");
                        }
                        catch (const std::exception& ex) {
                            logger.error("Error in websocket receive message~player_left." + string(ex.what()));
                        }
                    }
                    else if (broadcastMessage["type"] == "chat") {
                        std::cout << "[" << string(broadcastMessage["from"]) << "] " << string(broadcastMessage["speaker"]) << " >> " << string(broadcastMessage["sentence"]) << std::endl;
                    }
                }
                else if (message["type"] == "pushCommand") {
                    if (message["result"] == "success") {
                        int successAmount = message["successAmount"];
                        if (message["info"] == "ping") {
                            logger.info(string("Ping server: ") + std::to_string(successAmount) + string(" servers online."));
                        }
                        else {
                            if (message["sender"] == "daemon") {
                                if (successAmount > 0) {
                                    daemon.cure(message["info"]);
                                }
                            }
                            else {
                                logger.info(string("Command send to ") + std::to_string(successAmount) + string(" server(s)."));
                            }
                        }
                    }
                    else {
                        logger.error("Command send failed.");
                    }
                }
                ///// Request /////
                else {
                    logger.error("Can not analysis message: " + msg);
                }
            }
            catch (const std::exception& ex) {
                logger.error("Error in websocket receive message." + string(ex.what()));
                logger.error(msg);
            }
        }
        });
    ws.OnError([](cyanray::WebSocketClient& client, string msg) {
        logger.error("WebSocket connection error.");
        logger.error("Error message: " + msg);
        });
    ws.OnLostConnection([](cyanray::WebSocketClient& client, int   code) {
        logger.info("WebSocket connection lost. Error code: " + code);
        logger.info("Trying reconnect WebSocket server...");
        while (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
            Sleep(Config["web_socket"]["timeout"] * 1000);
            try {
                ws.Connect(Config["web_socket"]["host_url"]);
            }
            catch (const std::exception& ex) {
                logger.error("Can not connet websocket server." + string(ex.what()));
            }
        }
        });

    while (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
        Sleep(Config["web_socket"]["timeout"] * 1000);
        try {
            ws.Connect(Config["web_socket"]["host_url"]);
        }
        catch (const std::exception& ex) {
            logger.error("Can not connet websocket server." + string(ex.what()));
        }
    }
    // tick
    std::thread t1(thread_tick);

    // command
    while (true) {
        const int strSize = 999;
        char buffer[strSize];
        std::cin.getline(buffer, strSize);
        // cut
        vector<string> cmd;
        int i = 0;
        char c = buffer[i];
        string temp = "";

        while (true) {
            if (c == ' ') {
                cmd.push_back(temp);
                temp.clear();
            }
            else if (c == '\0') {
                cmd.push_back(temp);
                temp.clear();
                break;
            }
            else {
                temp += c;
            }
            i++;
            c = buffer[i];
        }
        for (string a : cmd) {
            logger.info(a);
        }
        // execute
        switch (hash_(cmd[0].c_str())) {
            case "quit"_hash: {
                logger.info("Stopping terminal..");
                t1.detach();
                logger.info("Quit correctly.");
                return 0;
            };
            case "stop"_hash: {
                if (cmd.size() == 2) {
                    WebAPI::pushCommand(cmd[1], "stop", "direct", "terminal", "default");
                }
                else {
                    logger.error("The server name is required in order to use the command \"stop\".");
                }
                break;
            }
            case "message"_hash: { // 8
            if (cmd.size() == 3) {
                string msg = buffer;
                switch (hash_(cmd[1].c_str())) {
                case "all"_hash: { //4
                    msg = msg.substr(12);
                    WebAPI::broadcastMessage_chat("sg", "terminal", msg);
                    break;
                }
                case "server"_hash: {// 7
                    msg = msg.substr(15);
                    WebAPI::broadcastMessage_chat("server", "terminal", msg);
                    break;
                }
                case "group"_hash: {// 6
                    msg = msg.substr(14);
                    WebAPI::broadcastMessage_chat("group", "terminal", msg);
                    break;
                }
                default:
                    break;
                }
            }
            default:
                break;
            }
        }
    }
    return 1;
}