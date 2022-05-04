#pragma once
#include <RegCommandAPI.h>
#include <DynamicCommandAPI.h>
#include <MC/ServerPlayer.hpp>
#include "WorldGroup.h"
#include "Language.h"
#include "PlayerTool.h"

/** CommandPermissionLevel 使用者的权限
*   Any = 0,            GameMasters = 1,    Admin = 2,
*   HostPlayer = 3,     Console = 4,        Internal = 5,
*/
/** CommandFlagValue       
*   None = 0,           Usage = 1,          Visibility2 = 2,
*   Visibility4 = 4,    Visibility6 = 6,    Sync = 8,
*   Execute = 16,       Type = 32,          Cheat = 64,
*/

class TeleportWorldCommand : public Command {
    CommandSelector<Player> player;
    string world = "";
    bool player_isSet;
    bool world_isSet;
public:
    void execute(CommandOrigin const& ori, CommandOutput& output) const override {
        bool user_is_player = false;
        ServerPlayer* sp = ori.getPlayer(); // 调用者(玩家)
        user_is_player = sp != nullptr;

        // tpw <player> <world> - teleport player to world
        if (world_isSet) {
            if (user_is_player && sp->getCommandPermissionLevel() < 1) {
                if (user_is_player)
                    sp->sendText(lang["command.info.noPermission"], TextType::RAW);
                else
                    logger.info(lang["command.info.noPermission"]);
            }
            else {
                CommandSelectorResults<Player> res = player.results(ori);
                if (Command::checkHasTargets(res, output) && (res.count() == 1)) {
                    if (serverName == world) {
                        if (user_is_player)
                            sp->sendText(lang["tpw.info.isSelf"], TextType::RAW);
                        else
                            logger.info(lang["tpw.info.isSelf"]);
                    }
                    // Teleport player(s)
                    else {
                        if (user_is_player)
                            sp->sendText(lang.transform("tpw.info.teleporting.user", std::to_string(res.data->size()), world), TextType::RAW);
                        nlohmann::json targets = nlohmann::json::array();
                        int i = 0;
                        for (Player* pl : *res.data) {
                            targets[i] = pl->getRealName();
                            i++;
                        }
                        if (user_is_player)
                            webAPI::tpw_setTransformingPlayer(sp->getXuid(), targets, world, "set");
                        else
                            webAPI::tpw_setTransformingPlayer(":console", targets, world, "set");
                    }
                }
            }
            return;
        }
        // tpw list - show world list
        else if (player.getName() == "list") {
            if(user_is_player) 
                webAPI::tpw_getWorldList(sp->getXuid());
            else
                webAPI::tpw_getWorldList(":console");
        }
        // tpw <world> - teleport to world
        // player.getName() is world name
        else{
            if (user_is_player){
                if (serverName == player.getName()) {
                    sp->sendText(lang["tpw.info.isSelf"], TextType::RAW);
                }
                // Teleport player
                else {
                    if (user_is_player) sp->sendText(lang["tpw.info.teleporting"], TextType::RAW);
                    webAPI::tpw_setTransformingPlayer(sp->getXuid(), nlohmann::json{sp->getRealName()}, player.getName(), "set");
                }
            }
            else {
                logger.info(lang["tpw.info.invalidUser"]);
            }
            return;
        }
    }

    static void setup(CommandRegistry* registry) {
        using RegisterCommandHelper::makeMandatory;
        using RegisterCommandHelper::makeOptional;
        registry->registerCommand("tpw", "tpw <world> - teleport to world",
            CommandPermissionLevel::Any, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
        registry->registerOverload<TeleportWorldCommand>("tpw", makeMandatory<CommandParameterDataType::NORMAL>(&TeleportWorldCommand::world, "world")
            );

        registry->registerCommand("tpw", "tpw <player> <world> - teleport player to world",
            CommandPermissionLevel::GameMasters, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
        registry->registerOverload<TeleportWorldCommand>("tpw",
            makeMandatory(&TeleportWorldCommand::player, "player", &TeleportWorldCommand::player_isSet),
            makeOptional(&TeleportWorldCommand::world,"world", &TeleportWorldCommand::world_isSet)
            );

        registry->registerCommand("tpw list", "tpw list - show world list",
            CommandPermissionLevel::Any, { (CommandFlagValue)0 }, { (CommandFlagValue)0x80 });
    }
};
// TODO: dynamic command
void dynamic() {
      // Direct setup of dynamic command with necessary information
      using ParamType = DynamicCommand::ParameterType;
      using Param = DynamicCommand::ParameterData;
      DynamicCommand::setup(
          "testenum",        // command name
          "dynamic command", // command description
          {
              // enums{enumName, {values...}}
              {"TestEnum1", {"add", "remove"}},
              {"TestEnum2", {"list"}},
          },
          {
              // parameters(type, name, [optional], [enumOptions(also enumName)], [identifier])
              // identifier: used to identify unique parameter data, if idnetifier is not set,
              //   it is set to be the same as enumOptions or name (identifier = enumOptions.empty() ? name:enumOptions)
              Param("testEnum", ParamType::Enum, "TestEnum1"),
              Param("testEnum", ParamType::Enum, "TestEnum2"),
              Param("testInt", ParamType::Int, true),
          },
          {
              // overloads{ (type == Enum ? enumOptions : name) ...}
              {"TestEnum1", "testInt"}, // testenum <add|remove> [testInt]
              {"TestEnum2"},            // testenum <list>
          },
          // dynamic command callback
          [](DynamicCommand const& command, CommandOrigin const& origin, CommandOutput& output,
          std::unordered_map<std::string, DynamicCommand::Result>& results) {
                  auto action = results["testEnum"].get<std::string>();
                  switch (do_hash(action.c_str()))
                  {
                  case do_hash("add"):
                      if (results["testInt"].isSet)
                          output.success(fmt::format("add {}", results["testInt"].getRaw<int>()));
                      else
                          output.success("add nothing");
                      break;
                  case do_hash("remove"):
                      if (results["testInt"].isSet)
                          output.success(fmt::format("remove {}", results["testInt"].getRaw<int>()));
                      else
                          output.success("remove nothing");
                      break;
                  case do_hash("list"):
                      output.success("list");
                      break;
                  default:
                      break;
                  }
          },
          CommandPermissionLevel::GameMasters);
}

void registerCommand(CommandRegistry* commandRegistry) {
    TeleportWorldCommand::setup(commandRegistry);
};