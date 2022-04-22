/* --------------------------------------- *\
 *  Name        :  Wheat                   *
 *  Description :  A server manage engine  *
 *  Version     :  alpha 0.2.1             *
 *  Author      :  ENIAC_Jushi             *
\* --------------------------------------- */

/* [Note]
 *
 * xuid、uuid 统一以字符串形式存储
 * 
 */ 

#include "pch.h"
#include "Config.h"
#include "webAPIs.h"
#include "Tools.h"

void webSocketReconnect();
void autoSave();

void PluginInit() {
    // ******** Config ******** //
    LL::registerPlugin("WheatManager", "A server manage engine.", LL::Version(0, 2, 1));
    loadConfig();
    logger.info(config_toString());

    // ******* WS Events ******* //
    ws.Connect(ws_hostURL);
    ws.OnTextReceived  ([](cyanray::WebSocketClient& client, string msg){
        if (msg.empty()) {
            return;
        }
        else if (msg == "success") {
            logger.info("Connection established.");
            webAPI::identityAuthentication("onServerStarted");
        }
        else {
            nlohmann::json message = nlohmann::json(msg);
            // Reply: Identity authentication
            if (message["type"] == "identityAuthentication") {
                if (message["result"] == "success")
                    logger.info("Authentication passed.");
                else 
                    logger.info("Authentication not passed.");
            }
            // Reply: onPreJoin
            if (message["info"] == "onPreJoin") {
                // Reply 0: Set player online status
                if (message["type"] == "setPlayerOnlineStatus") {
                    // 1 - Offline - //Reply 1//
                    if (message["result"] == "success") {
                        logger.info("Set player online status to " + serverName);
/*
  TODO: 检查超长字符串传输是否正常
*/
                        webAPI::getPlayerData(message["xuid"]);
                    }
                    // 2 - Online on this server - //End//
                    if (message["result"] == serverName) {
                        logger.info("Player is already online on this server.");
                        webAPI::setPlayerData(Level::getPlayer(to_string(message["xuid"])), "onPreJoin");
                    }
                    // 3 - Player doesn't exist - //Reply 2//
                    if (message["result"] == "void") {
                        logger.info("Couldn\'t find player online status.");
                        Player* pl = Level::getPlayer(to_string(message["xuid"]));
                        webAPI::bindGameCharacter(pl->getXuid(), pl->getUuid(), pl->getRealName(), "onPreJoin");
                    }
                    // 4 - Online on other server or other situations - //End//
                    else {
                        logger.info("Player is already online on server " + message["result"]);
                        Level::getPlayer(to_string(message["xuid"]))->kick("You're already online on another server:" + message["result"]);
                    }
                }
                // Reply 1: Get player data - //End// 
                // !! This is an unused reply
                if (message["type"] == "getPlayerData") {
                    if (message["result"] != "void") {
                        Player* pl = Level::getPlayer(to_string(message["xuid"]));
                        PlayerTool::setFromJson(pl, message);
                        pl->refreshInventory();
                    }
                }
                // Reply 2: Bind game character - //Reply 4//
                if (message["type"] == "bindGameCharacter") {
                    if (message["result"] == "success") {
                        webAPI::insertPlayerData(Level::getPlayer(to_string(message["xuid"])), "onPreJoin");
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
            // Request: broadcastMessage
            if (message["type"] == "broadcastMessage") {
                if (syn_message) {
                    nlohmann::json broadcastMessage = message["message"];
                    if (broadcastMessage["type"] == "chat") {
                        Level::broadcastText(string("[§b") + string(broadcastMessage["from"]) + string("§r]§g")
                            + string(broadcastMessage["speaker"]) + string("§r >> ") + string(broadcastMessage["sentence"]), TextType::RAW);
                        std::cout << "[" << broadcastMessage["from"] << "]" << broadcastMessage["speaker"] << " >> " << broadcastMessage["sentence"];
                    }
                    else if (broadcastMessage["type"] == "player_die") {
                        if (broadcastMessage["name"] == "null") {
                            Level::broadcastText(broadcastMessage["player"] + "失败了", TextType::RAW);
                        }
                        else {
                            Level::broadcastText(string(broadcastMessage["player"]) + string(" 被 ") 
                                + string(broadcastMessage["source"]) + string("打败了"), TextType::RAW);
                        }
                    }
                }
            }
        }
        });
    ws.OnError         ([](cyanray::WebSocketClient& client, string msg) {
        logger.error("WebSocket connection error.");
        logger.error("Error message: " + msg);
        });
    ws.OnLostConnection([](cyanray::WebSocketClient& client, int   code) {
        logger.info("WebSocket connection lost. Error code: " + code);
        });
    
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
        // identity authentication
        

        return true;
        });
    Event::PlayerPreJoinEvent::subscribe([](const Event::PlayerPreJoinEvent& ev) {
        // ev.mPlayer->sendText("hello world~");
        // ev.mXUID;
        // ev.mIP;
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) {
            ev.mPlayer->kick("Unable to reach web socket server, please contact with server administrator.");
        }
        else {
            webAPI::setPlayerOnlineStatus(ev.mPlayer->getXuid(), "login", "onPreJoin");
        }
        return true;
    });
    Event::PlayerJoinEvent   ::subscribe([](const Event::PlayerJoinEvent&    ev) {
        // 在某次修bug的时候挪了过来，当时认为是prejoin没有标签，但并没有验证到底需不需要
        if (syn_tags) {
            string tag = playerCache.getTags(ev.mPlayer->getXuid());
            if (tag != "false") {
                CompoundTag cTag;
                cTag.fromSNBT(tag);
                PlayerTool::setTags(*(ev.mPlayer->getNbt()), cTag);
            }
        }
        // TME会在onJoin时修改金钱，因此这里特别延迟改一次金钱
    // TODO: setplayerdata还没有加入缓存语句
        if (syn_money) {
            Sleep(1000);
            PlayerTool::setMoney(ev.mPlayer, playerCache.getMoney(ev.mPlayer->getXuid(), true));
        }
        return true;
        });
    Event::PlayerLeftEvent   ::subscribe([](const Event::PlayerLeftEvent&    ev) {
        // ev.mPlayer->sendText("hello world~");
        // ev.mXUID;
        if (!playerCache.findRefusedPlayer(ev.mXUID, true)) {
            webAPI::setPlayerOnlineStatus(ev.mXUID, "logout", "onLeft");
            webAPI::setPlayerData(ev.mPlayer, "onLeft");
        }
        // Delete cache for player who failed to connect.
        playerCache.getTags (ev.mXUID, true);
        playerCache.getMoney(ev.mXUID, true);
        return true;
        });
    Event::PlayerChatEvent   ::subscribe_ref([](  Event::PlayerChatEvent&    ev) {
        // ev.mMessage = "[Plugin Modified] " + ev.mMessage;
        webAPI::broadcastMessage_chat(ev.mPlayer->getRealName(), ev.mMessage);
        return true;
        });
    Event::PlayerDieEvent    ::subscribe_ref([](  Event::PlayerDieEvent&     ev) {
      // TODO: 找到Actor对应的死亡信息
        webAPI::broadcastMessage_die(ev.mPlayer->getRealName(),
            Level::getDamageSourceEntity(ev.mDamageSource)->getTypeName());
        return true;
        });
    
    // ******* Timmer ******* //
    std::thread webSocketReconnect(webSocketReconnect);
    std::thread autoSave(autoSave);

    logger.info("Loaded.");
}

void webSocketReconnect() {
    while (true) {
        if (ws.GetStatus() == cyanray::WebSocketClient::Status::Closed) {
            logger.info("WebSocket connection closed!");
            logger.info("Trying reconnect WebSocket server...");
            ws.Connect(ws_hostURL);
        }
        Sleep(ws_timeOut * 1000);
    }
}
void autoSave() {
    while (true) {
        logger.debug("Auto save");
        vector<Player*> playerList = Level::getAllPlayers();
        for (Player* player : playerList) {
            if (!playerCache.findRefusedPlayer(player->getXuid())) {
                webAPI::setPlayerData(player, "Auto save");
            }
        }
        Sleep(syn_autoSaveTime * 1000);
    }
}