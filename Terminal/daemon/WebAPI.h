#pragma once
#include "Config.h"

class WebAPI
{
public:
    static void identityAuthentication(string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "identityAuthentication"    },
            {"info"           ,  info                       },
            {"authentication" , "bot"                       },
            {"key"            ,  Config["web_socket"]["key"]},
            {"name"           ,  "end_point"                }
        };
        ws.SendText(message.dump());
    }
    static void broadcastMessage_chat(string channel, string speaker, string sentence) {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"   ,"broadcastMessage"},
            {"message",{
                {"type"     , "chat"     },
                {"channel"  , channel    },
                {"from"     , "Wheat"    },
                {"speaker"  , speaker    },
                {"sentence" , sentence   }
            }}
        };
        ws.SendText(message.dump());
    }
    static void pushCommand(string target, string command, string method, string sender, string info = "default") {
        if (ws.GetStatus() != cyanray::WebSocketClient::Status::Open) return;
        nlohmann::json message = nlohmann::json{
            {"type"           , "pushCommand"    },
            {"info"           ,  info            },
            {"sender"         ,  sender          },
            {"target"         ,  target          },
            {"method"         ,  method          },
            {"command"        ,  command         }
        };
        ws.SendText(message.dump());
    }
    static void pingServer(string target, string sender, string info) {
        WebAPI::pushCommand(target, " ", "ingnore", sender, info);
    }
};

