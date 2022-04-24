#pragma once
#include "Config.h"
#include "Tools.h"

class webAPI
{
public:
	static void identityAuthentication(string info = "default") {
        nlohmann::json message = nlohmann::json{
            {"type"           , "identityAuthentication"},
            {"authentication" , "server"                },
            {"key"            ,  ws_key                 },
            {"name"           ,  serverName             },
            {"info"           ,  info                   }
        };
        ws.SendText(message.dump());
	}
    static void bindGameCharacter(string xuid, string uuid, string realName, string info = "default") {
        nlohmann::json message = nlohmann::json{
            {"type"           , "bindGameCharacter"     },
            {"xuid"           ,  xuid                   },
            {"uuid"           ,  uuid                   },
            {"realName"       ,  realName               },
            {"info"           ,  info                   }
        };
        ws.SendText(message.dump());
    }
    static void setPlayerOnlineStatus(string xuid, string operate, string info = "default") {
        nlohmann::json message = nlohmann::json{
            {"type"           , "setPlayerOnlineStatus" },
            {"xuid"           ,  xuid                   },
            {"operate"        ,  operate                },
            {"info"           ,  info                   }
        };
        ws.SendText(message.dump());
    }
    static void setPlayerData(Player* pl, string info = "default") {
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
    static void insertPlayerData(Player* pl, string operate, string info = "default") {
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
    
    static void getPlayerData_HTML(const std::function<void(int, std::string)>& callback, string xuid) {
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
