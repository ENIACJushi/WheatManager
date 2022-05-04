#include <MC/ColorFormat.hpp>
#include "pch.h"
#include "Config.h"
#include "RegisterCommand.h"
#include "WebAPI.h"
#include "PlayerTool.h"
#include "MyHook.h"

/* --------------------------------------- *\
 *  Name        :  Wheat                   *
 *  Description :  A server manage engine  *
 *  Version     :  alpha 0.2.4             *
 *  Author      :  ENIAC_Jushi             *
\* --------------------------------------- */

/* [Note]
 *
 * xuid、uuid 统一以字符串形式存储
 *
 */


void PluginInit() {
    // ******** Config ******** //
    LL::registerPlugin("WheatManager", "A server manage engine.", LL::Version(0, 2, 4));
    loadConfig();
    logger.info(config_toString());

    // ******* WS Events ******* //
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
                    if (message["type"] == "getPlayerOnlineStatus") {
                        // 1 - Offline - //Reply 1//
                        if (message["result"] == "offline") {
                            logger.info("Player online status is offline.");
                            // webAPI::getPlayerData(message["xuid"], "onPreJoin");
                            webAPI::getPlayerData_HTTP([](int status, string data) {
                                if (status == 200) {
                                    nlohmann::json jsonData = json::parse(data.c_str(), nullptr, true);
                                    Player* pl = PlayerTool::getPlayerByXuid(jsonData["xuid"]);
                                    PlayerTool::setFromJson(pl, jsonData);
                                    if (setBagVersion == 1) {
                                        pl->refreshInventory();
                                    }
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
                else if (message["type"] == "getPlayerOnlineStatus") {
                    logger.debug("Player online status is: " + string(message["result"]));
                }
                else if (message["type"] == "setPlayerData") {
                    logger.debug("Change player cloud data: info: " + string(message["info"]) + string("; result: ") + string(message["result"]));
                }
                else if (message["type"] == "tpw_getWorldList") {
                    string listMsg = "";
                    if (message["result"] == "success") {
                        nlohmann::json worlds = message["worlds"];
                        if (worlds.size() == 0) {
                            listMsg = lang["tpw.info.worldList.none"];
                        }
                        else {
                            for (int i = 0; i < worlds.size(); i++) {
                                listMsg += worlds[i];
                                if (i != worlds.size() - 1)
                                    listMsg += ", ";
                            }
                        }
                    }
                    else {
                        listMsg = lang["tpw.info.worldList.failed"];
                    }
                    if (message["user"] == ":console") {
                        logger.info(listMsg);
                    }
                    else {
                        Player* pl = PlayerTool::getPlayerByXuid(message["user"]);
                        if (pl)
                            pl->sendText(listMsg, TextType::RAW);
                    }
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
                        else if (broadcastMessage["type"] == "player_join") {
                            try {
                                Level::broadcastText(string("{\"rawtext\":[{\"text\":\"" + ColorFormat::YELLOW + "\"},{\"translate\":\"") + string("multiplayer.player.joined")
                                    + string("\",\"with\":[\"") + string(broadcastMessage["player"]) + string("\"]}]}")
                                    , TextType::JSON);
                            }
                            catch (const std::exception& ex) {
                                logger.error("Error in websocket receive message~player_join." + string(ex.what()));
                            }
                        }
                        else if (broadcastMessage["type"] == "player_left") {
                            try {
                                Level::broadcastText(string("{\"rawtext\":[{\"text\":\"" + ColorFormat::YELLOW + "\"},{\"translate\":\"") + string("multiplayer.player.left")
                                    + string("\",\"with\":[\"") + string(broadcastMessage["player"]) + string("\"]}]}")
                                    , TextType::JSON);
                            }
                            catch (const std::exception& ex) {
                                logger.error("Error in websocket receive message~player_join." + string(ex.what()));
                            }
                        }
                        else if (broadcastMessage["type"] == "chat") {
                            Level::broadcastText(string("[§b") + string(broadcastMessage["from"]) + string("§r]§g")
                                + string(broadcastMessage["speaker"]) + string("§r >> ") + string(broadcastMessage["sentence"]), TextType::RAW);
                            std::cout << "[" << broadcastMessage["from"] << "]" << broadcastMessage["speaker"] << " >> " << broadcastMessage["sentence"] << std::endl;
                        }
                    }
                }
                else if (message["type"] == "runCommand") {
                    if (message["method"] == "direct") {
                        Level::runcmd(message["command"]);
                    }
                }
                else if (message["type"] == "tpw_setTransformingPlayer") {
                    // 本服务器作为起始服务器
                    if (message["info"] == "set") {
                        string userMsg = "";
                        // 目标服务器在线
                        if (message["result"] == "success") {
                            string successList = "";
                            nlohmann::json targetArray = message["targets"];
                            for (int i = 0; i < targetArray.size(); i++) {
                                Player* transformingPlayer = PlayerTool::getPlayerByRealName(targetArray[i]);
                                if (transformingPlayer) {
                                    transformingPlayer->sendText(lang["tpw.info.teleporting"], TextType::RAW);
                                    playerCache.pushTransformingPlayer(transformingPlayer->getRealName());
                                    if (transformingPlayer->transferServer(message["IP"], message["port"])) {
                                        successList += transformingPlayer->getRealName();
                                        if (i != targetArray.size() - 1) successList += ",";
                                    }
                                }
                            }
                            userMsg = lang.transform("tpw.info.teleporting.user", successList, message["destination"]);
                        }
                        else {
                            userMsg = lang["tpw.info.worldNotOnline"];
                        }
                        // 返回给调用者
                        if (message["user"] == ":console") {
                            logger.info(userMsg);
                        }
                        else {
                            Player* userPlayer = PlayerTool::getPlayerByXuid(message["user"]);
                            if(userPlayer)
                                userPlayer->sendText(userMsg, TextType::RAW);
                        }
                    }
                    // 本服务器作为目标服务器
                    else if (message["info"] == "command") {
                        for (int i = 0; i < message["targets"].size(); i++) {
                            playerCache.pushTransformingPlayer(message["targets"][i]);
                        }
                    }
                    else {
                        logger.warn("Invalid setTransformingPlayer message:" + msg);
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
        while (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
            Sleep(ws_timeOut * 1000);
            try {
                ws.Connect(ws_hostURL);
            }
            catch (const std::exception& ex) {
                logger.error("Can not connet websocket server." + string(ex.what()));
            }
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
        // connect web socket server
        while (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
            Sleep(ws_timeOut * 1000);
            try {
                ws.Connect(ws_hostURL);
            }
            catch (const std::exception& ex) {
                logger.error("Can not connet websocket server." + string(ex.what()));
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
            // webAPI::setPlayerOnlineStatus(ev.mPlayer->getXuid(), "login", "onPreJoin");
            webAPI::getPlayerOnlineStatus(ev.mPlayer->getXuid(), "onPreJoin");
        }
        return true;
    });
    Event::PlayerJoinEvent   ::subscribe([](const Event::PlayerJoinEvent&    ev) {
        std::pair<bool, string> isRefuse = playerCache.findRefusedPlayer(ev.mPlayer->getXuid());
        if (!isRefuse.first) {
            // 玩家准备进服时修改NBT概率失败，因此移到这里
            string newSNBT = playerCache.getNBT(ev.mPlayer->getXuid(), true);
            if (newSNBT != "null") {
                auto newNBT = CompoundTag::fromSNBT(newSNBT);
                ev.mPlayer->setNbt(newNBT.get());
                ev.mPlayer->refreshInventory();
            }
            // TME会在onJoin时修改金钱，因此这里特别延迟改一次金钱
            if (syn_money) {
                Sleep(1000);
                PlayerTool::setMoney(ev.mPlayer, playerCache.getMoney(ev.mPlayer->getXuid(), true));
            }
            // 完成一切同步后修改在线状态，并把玩家状态设为已同步
            webAPI::setPlayerOnlineStatus(ev.mPlayer->getXuid(), "login", "onJoin");
            playerCache.pushSynchronizedPlayer(ev.mPlayer->getXuid());
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
        bool refused = playerCache.findRefusedPlayer(ev.mXUID, true).first;
        bool synchronized = playerCache.findSynchronizedPlayer(ev.mXUID, true);
        if (!refused && synchronized) {
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
        // logger.debug("Auto save");
        vector<Player*> playerList = Level::getAllPlayers();
        for (Player* player : playerList) {
            // 玩家没有被拒绝且背包已经同步
            bool refused = playerCache.findRefusedPlayer(player->getXuid()).first;
            bool synchronized = playerCache.findSynchronizedPlayer(player->getXuid());
            if (!refused && synchronized) {
                webAPI::setPlayerData(player, "Auto save");
            }
        }
        }, syn_autoSaveTime * 1200);

    // 跨服传送0.5~1分钟未成功，判定失败
    Schedule::repeat([]() {
        if (playerCache.transformingPlayer.size() > 0) {
            for (std::list<std::pair<string, int>>::iterator id = playerCache.transformingPlayer.begin();
                id != playerCache.transformingPlayer.end(); id++) {
                id->second--;
                if (id->second <= 0) {
                    webAPI::broadcastMessage_left(id->first);
                    playerCache.transformingPlayer.erase(id);
                }
            }
        }
        }, 600);
}