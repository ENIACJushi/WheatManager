#pragma once
#include "Config.h"
#include "PlayerTool.h"
#include <Global.h>
#include <iostream>
class webAPI
{
public:
	static void identityAuthentication(string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "identityAuthentication"},
            {"info"           ,  info                   },
            {"authentication" , "server"                },
            {"key"            ,  ws_key                 },
            {"name"           ,  serverName             },
            {"IP"             ,  serverIP               },
            {"port"           ,  Global<PropertiesSettings>->getServerPort()},
        };
        ws.SendText(message.dump());
	}
    static void bindGameCharacter(string xuid, string uuid, string realName, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "bindGameCharacter"     },
            {"xuid"           ,  xuid                   },
            {"uuid"           ,  uuid                   },
            {"realName"       ,  realName               },
            {"info"           ,  info                   }
        };
        ws.SendText(message.dump());
    }
    static void getPlayerOnlineStatus(string xuid, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "getPlayerOnlineStatus" },
            {"xuid"           ,  xuid                   },
            {"info"           ,  info                   }
        };
        ws.SendText(message.dump());
    }
    static void setPlayerOnlineStatus(string xuid, string operate, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "setPlayerOnlineStatus" },
            {"xuid"           ,  xuid                   },
            {"operate"        ,  operate                },
            {"info"           ,  info                   }
        };
        ws.SendText(message.dump());
    }
    // targets: nlohmann::json{"name","name"...}
    static void tpw_setTransformingPlayer(string xuid, nlohmann::json targets, string serverName, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "tpw_setTransformingPlayer" },
            {"info"           ,  info                       },
            {"user"           ,  xuid                       },
            {"targets"        ,  targets                    }, //array
            {"destination"    ,  serverName                 }
        };
        ws.SendText(message.dump());
    }
    static void tpw_getWorldList(string xuid, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "tpw_getWorldList" },
            {"info"           ,  info              },
            {"user"           ,  xuid              }
        };
        ws.SendText(message.dump());
    }
    static void setPlayerData(Player* pl, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"        , "setPlayerData"         },
            {"xuid"        ,  pl->getXuid()          },
            {"bag"         ,  syn_bag        ? PlayerTool::getBag(pl)       ->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"enderChest"  ,  syn_enderChest ? PlayerTool::getEnderChest(pl)->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"attributes"  ,  syn_attributes ? PlayerTool::getAttributes(pl)->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"level"       ,  syn_level      ? PlayerTool::getLevel(pl)     ->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"tags"        ,  syn_tags       ? PlayerTool::getTags(pl)      ->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"scores"      ,  syn_scores     ? PlayerTool::getScores(pl).dump()                               : "false"},
            {"money"       ,  syn_money      ? PlayerTool::getMoney(pl)                                       : -9961  },
            {"info"        ,  info                   }
        };
        ws.SendText(message.dump());
    }
    static void insertPlayerData(Player* pl, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"        , "insertPlayerData" },
            {"xuid"        ,  pl->getXuid()          },
            {"bag"         ,  syn_bag        ? PlayerTool::getBag(pl)       ->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"enderChest"  ,  syn_enderChest ? PlayerTool::getEnderChest(pl)->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"attributes"  ,  syn_attributes ? PlayerTool::getAttributes(pl)->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"level"       ,  syn_level      ? PlayerTool::getLevel(pl)     ->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"tags"        ,  syn_tags       ? PlayerTool::getTags(pl)      ->toSNBT(0, SnbtFormat::Minimize) : "false"},
            {"scores"      ,  syn_scores     ? PlayerTool::getScores(pl).dump()                               : "false"},
            {"money"       ,  syn_money      ? PlayerTool::getMoney(pl)                                       : -9961  },
            {"info"        ,  info                   }
        };
        ws.SendText(message.dump());
    }
    static void getPlayerData(string xuid, string info="default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"        , "getPlayerData" },
            {"xuid"        ,  xuid           },
            {"info"        ,  info           },
            {"bag"         ,  syn_bag        },
            {"enderChest"  ,  syn_enderChest },
            {"attributes"  ,  syn_attributes },
            {"level"       ,  syn_level      },
            {"tags"        ,  syn_tags       },
            {"scores"      ,  syn_scores     },
            {"money"       ,  syn_money      },
        };
        ws.SendText(message.dump());
    }
    static void broadcastMessage_chat(string speaker, string sentence) {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"   ,"broadcastMessage"},
            {"message",{
                {"type"     , "chat"     },
                {"from"     , serverName },
                {"speaker"  , speaker    },
                {"sentence" , sentence   }
            }}
        };
        ws.SendText(message.dump());
    }
    static void broadcastMessage_die(string sentence, string player, string killer = "null", string tool = "null") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"   ,"broadcastMessage"},
            {"message",{
                {"type"     , "player_die" },
                {"from"     , serverName   },
                {"sentence" , sentence     },
                {"player"   , player       },
                {"killer"   , killer       },
                {"tool"     , tool         }
            }}
        };
        ws.SendText(message.dump());
    }
    static void broadcastMessage_join(string player, string info = "left") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"   ,"broadcastMessage"},
            {"message",{
                {"type"     , "player_join" },
                {"player"   , player    }
            }}
        };
        ws.SendText(message.dump());
    }
    static void broadcastMessage_left(string player, string info = "left") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"   ,"broadcastMessage"},
            {"message",{
                {"type"     , "player_left" },
                {"player"   , player    }
            }}
        };
        ws.SendText(message.dump());
    }
    static void getPlayerData_HTTP(const std::function<void(int, std::string)>& callback, string xuid) {
        HttpGet("http://" + hostURL + "/getPlayerData?xuid=" + xuid
                        + "&bag="        + (syn_bag        ? "true" : "false")
                        + "&enderChest=" + (syn_enderChest ? "true" : "false")
                        + "&attributes=" + (syn_attributes ? "true" : "false")
                        + "&level="      + (syn_level      ? "true" : "false")
                        + "&tags="       + (syn_tags       ? "true" : "false")
                        + "&scores="     + (syn_scores     ? "true" : "false")
                        + "&money="      + (syn_money      ? "true" : "false")
            , callback);
    }
};
