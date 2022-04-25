#include "pch.h"
#include "Config.h"
#include "RegisterCommand.h"
#include "webAPIs.h"
#include "Tools.h"

/* --------------------------------------- *\
 *  Name        :  Wheat                   *
 *  Description :  A server manage engine  *
 *  Version     :  alpha 0.2.2             *
 *  Author      :  ENIAC_Jushi             *
\* --------------------------------------- */

/* [Note]
 *
 * xuid、uuid 统一以字符串形式存储
 *
 */

void PluginInit() {
    // ******** Config ******** //
    LL::registerPlugin("WheatManager", "A server manage engine.", LL::Version(0, 2, 2));
    loadConfig();
    logger.info(config_toString());

    // ******* WS Events ******* //
    try {
        ws.Connect(ws_hostURL);
    }
    catch (const std::exception& ex) {
        logger.error("Can not connet websocket server." + string(ex.what()));
    }
    
    ws.OnTextReceived  ([](cyanray::WebSocketClient& client, string msg){
        // logger.info("[websocket] received:" + msg);
        if (msg.empty()) {
            return;
        }
        else if (msg == "success") {
            logger.info("Connection established.");
            webAPI::identityAuthentication("onServerStarted");
        }
        else if (!json::accept(msg)) {
            logger.info("[websocket] Invalid message received: " + msg);
        }
        else {
            try {
                const string m = msg;
                nlohmann::json message = json::parse(m.c_str(), nullptr, true);
                ///// Reply /////
                if (message["type"] == "identityAuthentication") {
                    if (message["result"] == "success")
                        logger.info("Authentication passed.");
                    else
                        logger.info("Authentication not passed.");
                }
                else if (message["info"] == "onPreJoin") {
                    // Reply 0: Set player online status
                    if (message["type"] == "setPlayerOnlineStatus") {
                        // 1 - Offline - //Reply 1//
                        if (message["result"] == "success") {
                            logger.info("Set player online status to " + serverName);
                            // webAPI::getPlayerData(message["xuid"], "onPreJoin");
                            webAPI::getPlayerData_HTML([](int status, string data) {
                                if (status == 200) {
                                    nlohmann::json jsonData = json::parse(data.c_str(), nullptr, true);
                                    Player* pl = PlayerTool::getPlayerByXuid(jsonData["xuid"]);
                                    PlayerTool::setFromJson(pl, jsonData);
                                    pl->refreshInventory();
                                }
                                else {
                                    logger.error("Get player data failed.");
                                }
                                }, message["xuid"]);
                        }
                        // 2 - Online on this server - //End//
                        else if (message["result"] == serverName) {
                            logger.info("Player is already online on this server.");
                            webAPI::setPlayerData(PlayerTool::getPlayerByXuid(message["xuid"]), "onPreJoin");
                        }
                        // 3 - Player doesn't exist - //Reply 2//
                        else if (message["result"] == "void") {
                            logger.info("Couldn\'t find player online status.");
                            Player* pl = PlayerTool::getPlayerByXuid(message["xuid"]);
                            webAPI::bindGameCharacter(string(pl->getXuid()), string(pl->getUuid()), pl->getRealName(), "onPreJoin");
                        }
                        // 4 - Online on other server or other situations - //End//
                        else {
                            playerCache.pushRefusedPlayer(string(message["xuid"]), "You're already online on another server: " + string(message["result"]));
                            logger.info("Player is already online on server " + string(message["result"]));
                            // 为正常显示踢出信息， 改到进入游戏再执行
                            // PlayerTool::getPlayerByXuid(message["xuid"])->kick("You're already online on another server:" + string(message["result"]));
                        }
                    }
                    // Reply 1: Get player data - //End// 
                    // !! This is an unused reply
                    if (message["type"] == "getPlayerData") {
                        if (message["result"] != "void") {
                            Player* pl = PlayerTool::getPlayerByXuid(message["xuid"]);
                            PlayerTool::setFromJson(pl, message);
                            pl->refreshInventory();
                        }
                    }
                    // Reply 2: Bind game character - //Reply 4//
                    if (message["type"] == "bindGameCharacter") {
                        if (message["result"] == "success") {
                            logger.info("Player profile build successfully.");
                            webAPI::insertPlayerData(PlayerTool::getPlayerByXuid(message["xuid"]), "onPreJoin");
                        }
                    }
                    // Reply 3: Set player Data - //End//
                    if (message["type"] == "setPlayerData") {
                        if (message["result"] == "success") {
                            logger.info("Success: Cloud data of player has been changed");
                        }
                        else {
                            logger.info("Falied: Cloud data of player has not been changed");
                        }
                    }
                    // Reply 4: Insert player Data
                    if (message["type"] == "insertPlayerData") {
                        if (message["result"] == "success") {
                            logger.info("Success: Cloud data of player has been insert.");
                        }
                        else {
                            logger.info("Falied: Cloud data of player has not been insert.");
                        }
                    }
                }
                else if (message["type"] == "setPlayerOnlineStatus") {
                    logger.debug("Set player online status: info: " + string(message["info"]) + string("; result: ") + string(message["result"]));
                }
                else if (message["type"] == "setPlayerData") {
                    logger.debug("Change player cloud data: info: " + string(message["info"]) + string("; result: ") + string(message["result"]));
                }
                ///// Request /////
                else if (message["type"] == "broadcastMessage") {
                    if (syn_message) {
                        nlohmann::json broadcastMessage = message["message"];
                        if (broadcastMessage["type"] == "player_die") {
                            try {
                                if (broadcastMessage["tool"] != "null") {
                                    // three parameter(player, killer, tool)
                                    Level::broadcastText(string("{\"rawtext\":[{\"translate\":\"") + string(broadcastMessage["sentence"])
                                        + string("\",\"with\":[\"") + string(broadcastMessage["player"])
                                        + string("\",\"") + string(broadcastMessage["killer"])
                                        + string("\",\"") + string(broadcastMessage["tool"]) + string("\"]}]}")
                                        , TextType::JSON);
                                }
                                else if (broadcastMessage["killer"] != "null") {
                                    // two parameter(player, killer)
                                    Level::broadcastText(string("{\"rawtext\":[{\"translate\":\"") + string(broadcastMessage["sentence"])
                                        + string("\",\"with\":[\"") + string(broadcastMessage["player"])
                                        + string("\",\"") + string(broadcastMessage["killer"]) + string("\"]}]}")
                                        , TextType::JSON);
                                }
                                else {
                                    // one parameter(player)
                                    Level::broadcastText(string("{\"rawtext\":[{\"translate\":\"") + string(broadcastMessage["sentence"])
                                        + string("\",\"with\":[\"") + string(broadcastMessage["player"]) + string("\"]}]}")
                                        , TextType::JSON);
                                }
                            }
                            catch (const std::exception& ex) {
                                logger.error("Error in websocket receive message~player_die." + string(ex.what()));
                            }
                        }
                        else if (broadcastMessage["type"] == "chat") {
                            Level::broadcastText(string("[§b") + string(broadcastMessage["from"]) + string("§r]§g")
                                + string(broadcastMessage["speaker"]) + string("§r >> ") + string(broadcastMessage["sentence"]), TextType::RAW);
                            std::cout << "[" << broadcastMessage["from"] << "]" << broadcastMessage["speaker"] << " >> " << broadcastMessage["sentence"];
                        }
                    }
                }
                else if (message["type"] == "runCommand") {
                    if (message["method"] == "direct") {
                        Level::runcmd(message["command"]);
                    }
                }
                else {
                    logger.warn("Can not analysis message: " + msg);
                }
            }
            catch (const std::exception& ex) {
                logger.warn("Error in websocket receive message."+ string(ex.what()));
                logger.warn(msg);
            }
        }
        });
    ws.OnError         ([](cyanray::WebSocketClient& client, string msg) {
        logger.error("WebSocket connection error.");
        logger.error("Error message: " + msg);
        });
    ws.OnLostConnection([](cyanray::WebSocketClient& client, int   code) {
        logger.info("WebSocket connection lost. Error code: " + code);
        logger.info("Trying reconnect WebSocket server...");
        try {
            ws.Connect(ws_hostURL);
        }
        catch (const std::exception& ex) {
            logger.error("Can not connet websocket server." + string(ex.what()));
        }
        });
    // TODO: 测试锁
    Schedule::repeat([]() {
        ws.SendText("Heart beat");
        }, 12000);
    // ******* MC Events ******* //
    Event::ServerStartedEvent::subscribe([](      Event::ServerStartedEvent  ev){
        // create money scoreboard
        if (syn_moneyType == "score") {
            Objective* result = nullptr;
            result = ::Global<Scoreboard>->getObjective(syn_moneyName);
            if (result == nullptr) {
                logger.info("Money scoreboard doesn't exist, create.");
                ::Global<Scoreboard>->newObjective(syn_moneyName, syn_moneyName);
            }
        }
        return true;
        });
    Event::RegCmdEvent       ::subscribe([](      Event::RegCmdEvent         ev) {
        registerCommand(ev.mCommandRegistry);
        return true;
        });
    Event::PlayerPreJoinEvent::subscribe([](const Event::PlayerPreJoinEvent& ev) {
        // ev.mPlayer->sendText("hello world~");
        // ev.mXUID;
        // ev.mIP;
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
            playerCache.pushRefusedPlayer(ev.mXUID, "Unable to reach web socket server, please contact with server administrator.");
        }
        else {
            webAPI::setPlayerOnlineStatus(ev.mPlayer->getXuid(), "login", "onPreJoin");
        }
        return true;
    });
    Event::PlayerJoinEvent   ::subscribe([](const Event::PlayerJoinEvent&    ev) {
        std::pair<bool, string> isRefuse = playerCache.findRefusedPlayer(ev.mPlayer->getXuid());
        if (!isRefuse.first) {
            // TME会在onJoin时修改金钱，因此这里特别延迟改一次金钱
            if (syn_money) {
                Sleep(1000);
                PlayerTool::setMoney(ev.mPlayer, playerCache.getMoney(ev.mPlayer->getXuid(), true));
            }
        }
        else {
            logger.info("kick " + ev.mPlayer->getRealName() + " " + isRefuse.second);
            Level::runcmd("kick " + ev.mPlayer->getRealName() + " " + isRefuse.second);
        }
        return true;
        });
    Event::PlayerLeftEvent   ::subscribe([](const Event::PlayerLeftEvent&    ev) {
        // ev.mPlayer->sendText("hello world~");
        // ev.mXUID;
        if (!playerCache.findRefusedPlayer(ev.mXUID, true).first) {
            webAPI::setPlayerOnlineStatus(ev.mXUID, "logout", "onLeft");
            webAPI::setPlayerData(ev.mPlayer, "onLeft");
        }
        // Delete cache for player who failed to connect.
        playerCache.getMoney(ev.mXUID, true);
        return true;
        });
    Event::PlayerChatEvent   ::subscribe_ref([](  Event::PlayerChatEvent&    ev) {
        // ev.mMessage = "[Plugin Modified] " + ev.mMessage;
        webAPI::broadcastMessage_chat(ev.mPlayer->getRealName(), ev.mMessage);
        return true;
        });
    Event::PlayerDieEvent    ::subscribe_ref([](  Event::PlayerDieEvent&     ev) {
        ActorDamageSource* ds = ev.mDamageSource;
        auto actors = ds->getDeathMessage("", ev.mPlayer);
        if (actors.second.size() == 1) {
            // one parameter(player)
            webAPI::broadcastMessage_die(actors.first, ev.mPlayer->getRealName());
        }
        else if (actors.second.size() == 2) {
            // two parameter(player, killer)
            webAPI::broadcastMessage_die(actors.first, ev.mPlayer->getRealName(), actors.second[1]);
        }
        else if (actors.second.size() == 3) {
            // three parameter(player, killer, tool)
            webAPI::broadcastMessage_die(actors.first, ev.mPlayer->getRealName(), actors.second[1], actors.second[2]);
        }
        return true;
        });
    
    // ******* Timmer ******* //
    // Auto save
    Schedule::repeat([]() {
        logger.debug("Auto save");
        vector<Player*> playerList = Level::getAllPlayers();
        for (Player* player : playerList) {
            if (!playerCache.findRefusedPlayer(player->getXuid()).first) {
                webAPI::setPlayerData(player, "Auto save");
            }
        }
        }, 1200 * syn_autoSaveTime);
}