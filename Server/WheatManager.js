
// ==================== Title ====================


 /* --------------------------------------- *\
  *  Name        :  Wheat                   *
  *  Description :  A server manage engine  *
  *  Version     :  alpha 0.3.0             *
  *  Author      :  ENIAC_Jushi             *
 \* --------------------------------------- */


const PLUGIN_PATH = "plugins/Wheat";
// ==================== Globle Values ===============
var PlayerData = {};
PlayerDataTool = {
    path: PLUGIN_PATH + "/player_data/",
    save:function(xuid){
        if(PlayerData[xuid] != null){
            file.writeTo(this.path + xuid + ".json", JSON.stringify(PlayerData[xuid], null , '\t'));
        }
    },
    load:function(xuid){
        var fileName = this.path + xuid + ".json";
        if(!file.exists(fileName))
            file.writeTo(fileName, JSON.stringify(defaultPlayerData, null , '\t'));
            PlayerData[xuid] = JSON.parse(file.readFrom(fileName));
    },
    release:function(xuid){
        if(PlayerData[xuid])
            delete PlayerData[xuid];
    }
}; // xuid:data
const defaultPlayerData = {
    "chat":{
        "channel":"sg",
        "mute"  : 0
    }
}
// ==================== Units =======================
////// Chat //////
var Chat = {
    setPlayerChannel:function(xuid, name){
        PlayerData[xuid]["chat"]["channel"] = name;
        PlayerDataTool.save(xuid);
    },
    getPlayerChannel:function(xuid){
        if(PlayerData[xuid]["chat"]["channel"]){
            return PlayerData[xuid]["chat"]["channel"];
        }
        else{
            PlayerData[xuid]["chat"]["channel"] = "sg";
            PlayerDataTool.save(xuid);
            return "sg";
        }
    },
    getPlayerMuteTime:function(xuid){
        if(PlayerData[xuid]["chat"]["mute"]){
            return PlayerData[xuid]["chat"]["mute"];
        }
        else{
            PlayerData[xuid]["chat"]["mute"] = 0;
            PlayerDataTool.save(xuid);
            return 0;
        }
    },
    setPlayerMuteTime:function(xuid, time){
        if(PlayerData[xuid]["chat"]["mute"]){
            PlayerData[xuid]["chat"]["mute"] = time;
        }
        else{
            PlayerData[xuid]["chat"]["mute"] = time;
            PlayerDataTool.save(xuid);
        }
    },
    decreasePlayerMuteTime:function(xuid, time=1){
        if(PlayerData[xuid]["chat"]["mute"] != null){
            PlayerData[xuid]["chat"]["mute"] -= time;
        }
        else{
            PlayerData[xuid]["chat"]["mute"] = 0;
            PlayerDataTool.save(xuid);
        }
    },
    translateChatString:function(from, name, sentence){
        var str = "[§b{F}§r] §g{N}§r >> {S}";
        str = str.replace(`{F}`, from);
        str = str.replace(`{N}`, name);
        str = str.replace(`{S}`, sentence);
        return str;
    },
    sendChannelMessage:function(channel, message){
        var playerList = mc.getOnlinePlayers();
        for (var pl of playerList) {
           if(channel == "sg" || channel == "server"){
               pl.sendText(message, 0);
           }
           else if(Chat.getPlayerChannel(pl.xuid) == channel){
              pl.sendText(message, 0);
           }
        }
    }
}

// ==================== Config ======================
var languageName, hostURL, serverName, serverIP, serverPort,
    ws, ws_hostURL, ws_key, ws_timeOut,
    syn_autoSaveTime, syn_bag, syn_enderChest, syn_attributes, syn_level,
    syn_scores, syn_tags,syn_message,syn_money,syn_moneyType,syn_moneyName;
Config = {
    load:function(){
        // 判断文件是否存在, 不存在则以默认配置创建
        if(!file.exists("plugins/Wheat/config.json")) {
            file.writeTo("plugins/Wheat/config.json", JSON.stringify({
                "language":{
                    "name"         : "zh_CN"
                },
                "Base":{
                    "hostURL"      : "localhost"      ,
                    "serverName"   : "world"          ,
                    "serverIP"     : "localhost"      ,
                    "serverPort"   : 9961             ,
                }                                     ,
                "WebSocket":{
                    "hostURL"      : "ws://localhost/ws/asset" ,
                    "key"          : "key"            ,
                    "timeOut"      : 15               ,
                }                                     ,
                "Synchronize":{
                    "autoSaveTime" : 5                ,
                    "bag"          : true             ,
                    "enderChest"   : true             ,
                    "attributes"   : true             ,
                    "level"        : true             ,
                    "scores"       : true             ,
                    "tags"         : true             ,
                    "message"      : true             ,
                    "money"        : true             ,
                    "moneyType"    : "score"          ,
                    "moneyName"    : "money"
                }
            } , null , '\t'));
            setTimeout(() => { logger.info('未找到配置文件，以默认配置启动'); }, 6000);
        }
        Config = JSON.parse(file.readFrom("plugins/Wheat/config.json"));
        // Other plugins: llmoney使用LLSEapi
        // Debug
        setBagVersion = 2;
        // Language
        languageName     = Config[ "language"     ][ "name"            ];
        // Base
        hostURL          = Config[ "Base"         ][ "hostURL"         ];
        serverName       = Config[ "Base"         ][ "serverName"      ];
        serverIP         = Config[ "Base"         ][ "serverIP"        ];
        serverPort       = Config[ "Base"         ][ "serverPort"      ];
        // Web socket
        ws               = new WSClient();
        ws_hostURL       = Config[ "WebSocket"     ][ "hostURL"        ];
        ws_key           = Config[ "WebSocket"     ][ "key"            ];
        ws_timeOut       = Config[ "WebSocket"     ][ "timeOut"        ];
        // Synchronize
        syn_autoSaveTime = Config[ "Synchronize"   ][ "autoSaveTime"   ];
        syn_bag          = Config[ "Synchronize"   ][ "bag"            ];
        syn_enderChest   = Config[ "Synchronize"   ][ "enderChest"     ];
        syn_attributes   = Config[ "Synchronize"   ][ "attributes"     ];
        syn_level        = Config[ "Synchronize"   ][ "level"          ];
        syn_scores       = Config[ "Synchronize"   ][ "scores"         ];
        syn_tags         = Config[ "Synchronize"   ][ "tags"           ];
        syn_message      = Config[ "Synchronize"   ][ "message"        ];
        syn_money        = Config[ "Synchronize"   ][ "money"          ];
        syn_moneyType    = Config[ "Synchronize"   ][ "moneyType"      ];
        syn_moneyName    = Config[ "Synchronize"   ][ "moneyName"      ];
    },
    toString:function(){
        return "Loading...\n\n   =====================================\n\t[Base]\n\t  hostURL:\t" + hostURL + "\n"
        + "\t  serverName:\t"   + serverName                          + "\n"
        + "\t  serverIP:\t"     + serverIP                            + "\n"
        + "\t  language:\t"     + languageName                        + "\n"
        + "\t[Web Socket]\n"
        + "\t  host URL:\t"     + ws_hostURL                          + "\n"
        + "\t  key:\t\t"        + ws_key                              + "\n"
        + "\t  timeOut:\t"      + ws_timeOut                          + "\n"
        + "\t[Synchronization]\n"
        + "\t  autoSaveTime:\t" + syn_autoSaveTime                    + "\n"
        + "\t  bag:\t\t"        + (syn_bag        ? "true" : "false") + "\n"
        + "\t  enderChest:\t"   + (syn_enderChest ? "true" : "false") + "\n"
        + "\t  attributes:\t"   + (syn_attributes ? "true" : "false") + "\n"
        + "\t  level:\t"        + (syn_level      ? "true" : "false") + "\n"
        + "\t  scores:\t"       + (syn_scores     ? "true" : "false") + "\n"
        + "\t  tags:\t\t"       + (syn_tags       ? "true" : "false") + "\n"
        + "\t  message:\t"      + (syn_message    ? "true" : "false") + "\n"
        + "\t  money:\t"        + (syn_money      ? "true" : "false") + "\n"
        + "\t  money type:\t"   + syn_moneyType                       + "\n"
        + "\t  money name:\t"   + syn_moneyName                       + "\n"
        + "   =====================================\n";
    }
}
// ==================== Language ====================
// var language = {
//     m:new Map(),
//     load:function(){
//         var str = file.readFrom("plugins/Wheat/texts/" + languageName + ".lang");

//     },
//     transform:function(key, t1="", t2="", t3="", t4=""){
//         var str = this.valueOf(key);
//         str = str.replace("%1", t1);
//         str = str.replace("%2", t2);
//         str = str.replace("%3", t3);
//         str = str.replace("%4", t4);
//         return str;
//     },
//     valueOf:function(key){
//         return this.m.get('key');
//     }
// }
var language;
// ==================== Web API =====================
WebAPI = {
    identityAuthentication:function(info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"           : "identityAuthentication",
            "info"           :  info                   ,
            "authentication" : "server"                ,
            "key"            :  ws_key                 ,
            "name"           :  serverName             ,
            "IP"             :  serverIP               ,
            "port"           :  serverPort             ,
        };
        ws.send(JSON.stringify(message));
	},
    bindGameCharacter:function(xuid, uuid, realName, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"           : "bindGameCharacter"     ,
            "xuid"           :  xuid                   ,
            "uuid"           :  uuid                   ,
            "realName"       :  realName               ,
            "info"           :  info                   
        };
        ws.send(JSON.stringify(message));
    },
    getPlayerOnlineStatus:function(xuid, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"           : "getPlayerOnlineStatus" ,
            "xuid"           :  xuid                   ,
            "info"           :  info                   
        };
        ws.send(JSON.stringify(message));
    },
    setPlayerOnlineStatus:function(xuid, operate, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"           : "setPlayerOnlineStatus" ,
            "xuid"           :  xuid                   ,
            "operate"        :  operate                ,
            "info"           :  info                   
        };
        ws.send(JSON.stringify(message));
    },
    // targets: {"name","name"...}
    tpw_setTransformingPlayer:function(xuid, targets, serverName, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"           : "tpw_setTransformingPlayer" ,
            "info"           :  info                       ,
            "user"           :  xuid                       ,
            "targets"        :  targets                    , //array
            "destination"    :  serverName                 
        };
        ws.send(JSON.stringify(message));
    },
    tpw_getWorldList:function(xuid, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"           : "tpw_getWorldList" ,
            "info"           :  info              ,
            "user"           :  xuid              
        };
        ws.send(JSON.stringify(message));
    },
    setPlayerData:function(pl, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"        : "setPlayerData" ,
            "xuid"        :  pl.xuid   ,
            "bag"         :  syn_bag        ? PlayerTool.getBag       (pl).toSNBT(0)   : "false",
            "enderChest"  :  syn_enderChest ? PlayerTool.getEnderChest(pl).toSNBT(0)   : "false",
            "attributes"  :  syn_attributes ? PlayerTool.getAttributes(pl).toSNBT(0)   : "false",
            "level"       :  syn_level      ? PlayerTool.getLevel     (pl).toSNBT(0)   : "false",
            "tags"        :  syn_tags       ? PlayerTool.getTags      (pl).toSNBT(0)   : "false",
            "scores"      :  syn_scores     ? JSON.stringify(PlayerTool.getScores(pl)) : "false",
            "money"       :  syn_money      ? PlayerTool.getMoney     (pl)             : -9961  ,
            "info"        :  info
        };
        ws.send(JSON.stringify(message));
    },
    insertPlayerData:function(pl, info = "default") {
        if (ws.status == 2) return;
        var message = {
            "type"        : "insertPlayerData" ,
            "xuid"        :  pl.xuid      ,
            "bag"         :  syn_bag        ? PlayerTool.getBag       (pl).toSNBT(0)   : "false",
            "enderChest"  :  syn_enderChest ? PlayerTool.getEnderChest(pl).toSNBT(0)   : "false",
            "attributes"  :  syn_attributes ? PlayerTool.getAttributes(pl).toSNBT(0)   : "false",
            "level"       :  syn_level      ? PlayerTool.getLevel     (pl).toSNBT(0)   : "false",
            "tags"        :  syn_tags       ? PlayerTool.getTags      (pl).toSNBT(0)   : "false",
            "scores"      :  syn_scores     ? JSON.stringify(PlayerTool.getScores(pl)) : "false",
            "money"       :  syn_money      ? PlayerTool.getMoney     (pl)             : -9961  ,
            "info"        :  info
        };
        ws.send(JSON.stringify(message));
    },
    getPlayerData:function(xuid, info="default") {
        if (ws.status == 2) return;
        var message = {
            "type"        : "getPlayerData" ,
            "xuid"        :  xuid           ,
            "info"        :  info           ,
            "bag"         :  syn_bag        ,
            "enderChest"  :  syn_enderChest ,
            "attributes"  :  syn_attributes ,
            "level"       :  syn_level      ,
            "tags"        :  syn_tags       ,
            "scores"      :  syn_scores     ,
            "money"       :  syn_money
        };
        ws.send(JSON.stringify(message));
    },
    broadcastMessage_chat:function(speaker, sentence, channel) {
        if (ws.status == 2) return;
        var message = {
            "type"   :"broadcastMessage",
            "message":{
                "type"     : "chat"     ,
                "channel"   : channel     ,
                "from"     : serverName ,
                "speaker"  : speaker    ,
                "sentence" : sentence   
            }
        };
        ws.send(JSON.stringify(message));
    },
    broadcastMessage_die:function(sentence, player, killer = "null", tool = "null") {
        if (ws.status == 2) return;
        var message = {
            "type"   :"broadcastMessage",
            "message":{
                "type"     : "player_die" ,
                "from"     : serverName   ,
                "sentence" : sentence     ,
                "player"   : player       ,
                "killer"   : killer       ,
                "tool"     : tool         
            }
        };
        ws.send(JSON.stringify(message));
    },
    broadcastMessage_join:function(player, info = "left") {
        if (ws.status == 2) return;
        var message = {
            "type"   :"broadcastMessage",
            "message":{
                "type"     : "player_join" ,
                "player"   : player    
            }
        };
        ws.send(JSON.stringify(message));
    },
    broadcastMessage_left:function(player, info = "left") {
        if (ws.status == 2) return;
        var message = {
            "type"   :"broadcastMessage",
            "message":{
                "type"     : "player_left" ,
                "player"   : player    
            }
        };
        ws.send(JSON.stringify(message));
    },
    getPlayerData_HTTP:function(xuid ,callback) {
        var requestURL = "http://" + hostURL + "/getPlayerData?xuid=" + xuid
                        + "&bag="        + (syn_bag        ? "true" : "false")
                        + "&enderChest=" + (syn_enderChest ? "true" : "false")
                        + "&attributes=" + (syn_attributes ? "true" : "false")
                        + "&level="      + (syn_level      ? "true" : "false")
                        + "&tags="       + (syn_tags       ? "true" : "false")
                        + "&scores="     + (syn_scores     ? "true" : "false")
                        + "&money="      + (syn_money      ? "true" : "false");
        network.httpGet(requestURL, (status, data) => {
            if (status == 200) {
                var jsonData = JSON.parse(data);
                var pl = PlayerTool.getPlayerByXuid(jsonData["xuid"]);
                PlayerTool.setFromJson_delay(pl, jsonData);
            }
            else {
                logger.error("Get player data failed.");
            }
        });
        
    },
    setPlayerData_HTTP:function(pl, info, callback) {
        var requestURL = "http://" + hostURL + "/setPlayerData?xuid=" + pl.xuid
            + "&key="        + ws_key
            + "&info="       + info
            + "&bag="        + (syn_bag        ? PlayerTool.getBag       (pl).toSNBT(0)   : "false")
            + "&enderChest=" + (syn_enderChest ? PlayerTool.getEnderChest(pl).toSNBT(0)   : "false")
            + "&attributes=" + (syn_attributes ? PlayerTool.getAttributes(pl).toSNBT(0)   : "false")
            + "&level="      + (syn_level      ? PlayerTool.getLevel     (pl).toSNBT(0)   : "false")
            + "&tags="       + (syn_tags       ? PlayerTool.getTags      (pl).toSNBT(0)   : "false")
            + "&scores="     + (syn_scores     ? JSON.stringify(PlayerTool.getScores(pl)) : "false")
            + "&money="      + (syn_money      ? PlayerTool.getMoney     (pl)             : -9961  );
        
        network.httpGet(requestURL, (status, result) => {
            
        });
    }
}
// ==================== Player Tool =================
PlayerCache = {
    money               : {}, // (xuid - money),(xuid - money)
    refusedPlayer       : {}, // (xuid-serverName), (xuid-serverName)...
    synchronizedPlayer  : [], // xuid, xuid...
    transformingPlayer  : {}, // (name-timestamp), (name-timestamp)... 因为拦截时按name拦截
    sNBT                : [], // (xuid - NBTMessageJson)
    pushMoney:function(xuid, m) {
        this.money[xuid] = m;
	},
    // if exist, return serverName, or return null.
	getMoney:function(xuid, del = false) {
        if(this.money[xuid]){
            if(del){
                var result = this.money[xuid];
                delete this.money[xuid];
                return result;
            }
            else{
                return this.money[xuid];
            }
        }
        return null;
	},

    pushRefusedPlayer:function(xuid, serverName) {
        this.refusedPlayer[xuid] = serverName;
	},
    // if exist, return serverName, or return null.
	findRefusedPlayer:function(xuid, del = false) {
        if(this.refusedPlayer[xuid]){
            if(del){
                var result = this.refusedPlayer[xuid];
                delete this.refusedPlayer[xuid];
                return result;
            }
            else{
                return this.refusedPlayer[xuid];
            }
        }
        return null;
	},

	pushSynchronizedPlayer:function(xuid) {
		this.synchronizedPlayer.push(xuid);
	},
    // if exist, return ture, or return false.
	findSynchronizedPlayer:function(xuid, del = false) {
        for (var i in this.synchronizedPlayer) {
            if(this.synchronizedPlayer[i] == xuid){
                if(del) delete this.synchronizedPlayer[i];
                return true;
            }
        }
		return false;
	},

	pushTransformingPlayer:function(name) {
		this.transformingPlayer[name] = 2;
	},
    
	findTransformingPlayer:function(name, del = false) {
		if(this.transformingPlayer[name]){
            if(del){
                var result = this.transformingPlayer[name];
                delete this.transformingPlayer[name];
                return result;
            }
            else{
                return this.transformingPlayer[name];
            }
        }
        return null;
	},

    pushNBT:function(xuid, NBT) {
        this.sNBT[xuid] = NBT;
	},
    // if exist, return serverName, or return null.
	getNBT:function(xuid, del = false) {
        if(this.sNBT[xuid]){
            if(del){
                var result = this.sNBT[xuid];
                delete this.sNBT[xuid];
                return result;
            }
            else{
                return this.sNBT[xuid];
            }
        }
        return null;
	},
}
PlayerTool = {
    //// Get ////
	getPlayerByXuid:function(xuid) {
		var playerList = mc.getOnlinePlayers();
        for(var i in playerList) {
            if (playerList[i].xuid == xuid) {
				return playerList[i];
			}
        }
		return null;
	},
	getPlayerByRealName:function(name) {
		var playerList = mc.getOnlinePlayers();
        for(var i in playerList) {
            if (playerList[i].realName == xuid) {
				return playerList[i];
			}
        }
		return null;
	},
	getBag:function(pl) {
        var playerNBT     = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'Armor'               : playerNBT.getTag( 'Armor'               ),
            'Inventory'           : playerNBT.getTag( 'Inventory'           ),
            'Mainhand'            : playerNBT.getTag( 'Mainhand'            ),
            'Offhand'             : playerNBT.getTag( 'Offhand'             )
        });
        playerNBT.destroy();
        return playerNBTData;
	},
	getEnderChest:function(pl) {
        var playerNBT     = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'EnderChestInventory' : playerNBT.getTag( 'EnderChestInventory' )
        });
        playerNBT.destroy();
        return playerNBTData;
	},
	getAttributes:function(pl) {
        var playerNBT     = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'Attributes'          : playerNBT.getTag( 'Attributes'          )
        });
        playerNBT.destroy();
        return playerNBTData;
	},
	getLevel:function(pl) {
        var playerNBT     = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'PlayerLevel'         : playerNBT.getTag( 'PlayerLevel'         ),
            'PlayerLevelProgress' : playerNBT.getTag( 'PlayerLevelProgress' )
        });
        playerNBT.destroy();
        return playerNBTData;
	},
	getTags:function(pl) {
        var playerNBT     = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'Tags'                : playerNBT.getTag( 'Tags'                )
        });
        playerNBT.destroy();
        return playerNBTData;
	},
	getScores:function(pl) {
        var scoreboard = mc.getAllScoreObjectives();
        var scores = {};
        for(var i in scoreboard) {
            scores[scoreboard[i].name] = pl.getScore(scoreboard[i].name);
        }
        return scores;
	},
	getMoney:function(pl) {
		if (syn_moneyType == "score") {
			return pl.getScore(syn_moneyName);
		}
		else if (syn_moneyType == "LLMoney") {
			return money.get(pl.xuid);
		}
		else {
			logger.error("Failed to get player data: money. Invalid money type.(\'score\' or \'LLMoney\')");
			return -9961;
		}
	},
	//// Set ////
    // TODO: 延迟踢出
	setBag:function(pl, NBT) {
        if(NBT == null){
            logger.info(`Set player data successfully: ${pl.realName}, NBT, not set.`);
        }
        else{
            var playerNBT = pl.getNbt();
            playerNBT.setTag( "Armor"               , NBT.getTag( "Armor"               ));
            playerNBT.setTag( "Inventory"           , NBT.getTag( "Inventory"           ));
            playerNBT.setTag( "Mainhand"            , NBT.getTag( "Mainhand"            ));
            playerNBT.setTag( "Offhand"             , NBT.getTag( "Offhand"             ));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                logger.info(`Set player data successfully: ${pl.realName}, Bag.`);
            }
            else {
                logger.error(`Failed to set player data: ${pl.realName}, Bag`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [WM] Bag set failed.`);
            }
            NBT.destroy();
            playerNBT.destroy();
        }
	},
	setEnderChest:function(pl, NBT) {
        if(NBT == null){
            logger.info(`Set player data successfully: ${pl.realName}, EnderChest, not set.`);
        }
        else{
            var playerNBT = pl.getNbt();
            playerNBT.setTag( "EnderChestInventory" , NBT.getTag( "EnderChestInventory" ));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                logger.info(`Set player data successfully: ${pl.realName}, EnderChest.`);
            }
            else {
                logger.error(`Failed to set player data: ${pl.realName}, EnderChest`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [WM] EnderChest set failed.`);
            }
            NBT.destroy();
            playerNBT.destroy();
        }
	},
	setAttributes:function(pl, NBT) {
        if(NBT == null){
            logger.info(`Set player data successfully: ${pl.realName}, Attributes, not set.`);
        }
        else{
            var playerNBT = pl.getNbt();
            playerNBT.setTag( "Attributes"          , NBT.getTag( "Attributes"          ));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                logger.info(`Set player data successfully: ${pl.realName}, Attributes.`);
            }
            else {
                logger.error(`Failed to set player data: ${pl.realName}, Attributes`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [WM] Attributes set failed.`);
            }
            NBT.destroy();
            playerNBT.destroy();
        }
	},
	setLevel:function(pl, NBT) {
        if(NBT == null){
            logger.info(`Set player data successfully: ${pl.realName}, PlayerLevel, not set.`);
        }
        else{
            var playerNBT = pl.getNbt();
            playerNBT.setTag( "PlayerLevel"         , NBT.getTag( "PlayerLevel"         ));
            playerNBT.setTag( "PlayerLevelProgress" , NBT.getTag( "PlayerLevelProgress" ));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                logger.info(`Set player data successfully: ${pl.realName}, PlayerLevel.`);
            }
            else {
                logger.error(`Failed to set player data: ${pl.realName}, PlayerLevel`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [WM] PlayerLevel set failed.`);
            }
            NBT.destroy();
            playerNBT.destroy();
        }
	},
	setTags:function(pl, NBT) {
        if(NBT == null){
            logger.info(`Set player data successfully: ${pl.realName}, PlayerLevel, not set.`);
        }
        else{
            var playerNBT = pl.getNbt();
            playerNBT.setTag( "Tags"                , NBT.getTag( "Tags"                ));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                logger.info(`Set player data successfully: ${pl.realName}, PlayerLevel.`);
            }
            else {
                logger.error(`Failed to set player data: ${pl.realName}, PlayerLevel`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [WM] PlayerLevel set failed.`);
            }
            NBT.destroy();
            playerNBT.destroy();
        }
	},
	setScores:function(pl, scores) {
        if(scores == undefined){
            logger.info(`Set player data successfully: ${pl.realName}, scoreboard, not set.`);
        }
        else{
            for(var i in scores) {
                if (mc.getScoreObjective(i) == undefined) {
                    mc.newScoreObjective(i, i);
                }
                pl.setScore(i, scores[i]);
            }
            logger.info(`Set player data successfully: ${pl.realName}, scoreboard.`);
        }
	},
	setMoney:function(pl, money) {
        if(money == null || money == -9961){
            logger.info(`Set player data successfully: ${pl.realName}, money, not set.`);
        }
        else{
            if(syn_moneyType == "score"){
                pl.setScore(syn_moneyName, money);
                logger.info(`Set player data successfully: ${pl.realName}, money, ${money}.`);
            }
            else if(syn_moneyType == "LLMoney"){
                money.set(pl.xuid, money);
                logger.info(`Set player data successfully: ${pl.realName}, money, ${money}.`);
            }
            else{
                logger.error("Failed to set player data: money.\n  Invalid money type.(\'score\' or \'LLMoney\')")
            }
        }
	},
	setFromJson:function(pl, playerDataJson) {
        if (syn_bag && playerDataJson["bag"] != "false") {
            this.setBag(pl, NBT.parseSNBT(playerDataJson["bag"]));
        }
        if (syn_enderChest && playerDataJson["enderChest"] != "false") {
            this.setEnderChest(pl, NBT.parseSNBT(playerDataJson["enderChest"]));
        }
        if (syn_attributes && playerDataJson["attributes"] != "false") {
            this.setAttributes(pl, NBT.parseSNBT(playerDataJson["attributes"]));
        }
        if (syn_level && playerDataJson["level"] != "false") {
            this.setLevel(pl, NBT.parseSNBT(playerDataJson["level"]));
        }
        if (syn_tags && playerDataJson["tags"] != "false") {
            this.setTags(pl, NBT.parseSNBT(playerDataJson["tags"]));
        }
        if (syn_scores && playerDataJson["scores"] != "false") {
            this.setScores(pl, JSON.parse(playerDataJson["scores"]));
        }
        if (syn_money && playerDataJson["money"] != -9961) {
            this.setMoney(pl, playerDataJson["money"]);
        }
        pl.refreshItems();
        return true;
	},
    setFromJson_delay:function(pl, playerDataJson){
        PlayerCache.pushNBT(pl.xuid, playerDataJson);
        PlayerCache.pushMoney(pl.xuid, playerDataJson["money"]);
    }
}
// ==================== Initialize ==================
let version = 0.1
// log output
logger.setConsole(true);
logger.setTitle('Wheat');
logger.info('Wheat is running');
// floder
if(!file.exists("plugins/Wheat")) {
    file.mkdir("plugins/Wheat");
}
// Config
Config.load();
logger.info(Config.toString());
///// Register Commands /////
CommandManager = {
    set:function(){
        this.getserver();
        this.chat();
        this.chatop();
        this.mute();
    },
    getserver:function(){
        let cmd = mc.newCommand("getserver", "get server name.", PermType.Any);
        cmd.overload ();
        cmd.setCallback((_cmd, _ori, out, res) => {
            return out.success(`The name of this server is: "${serverName}"`);
        });
        cmd.setup();
    },
    chat:function(){
        let cmd = mc.newCommand("chat", "Wheat Builder", PermType.Any);
        /// Show
        cmd.setEnum  ("e_show"         , ["show"  ]);
        cmd.mandatory("show"   , ParamType.Enum, "e_show"  , 1 );
        /// Switch
        cmd.setEnum  ("e_switch"     , ["switch"       ]);
        cmd.mandatory("switch"       , ParamType.Enum, "e_switch"  , 1 );
        // server
        cmd.setEnum  ("e_server"       , ["server" ]);
        cmd.mandatory("channel_server" , ParamType.Enum, "e_server", 1 );
        // server & group
        cmd.setEnum  ("e_sg"           , ["sg"     ]);
        cmd.mandatory("channel_sg"     , ParamType.Enum, "e_sg"    , 1 );
        // custom
        cmd.setEnum  ("e_custom"       , ["custom" ]);
        cmd.mandatory("channel_custom" , ParamType.Enum, "e_custom", 1 );
        cmd.mandatory("channel_name"    , ParamType.String              );
        // switch commands
        cmd.overload (["show"]);
        cmd.overload (["switch", "channel_server"]);
        cmd.overload (["switch", "channel_sg"    ]);
        cmd.overload (["switch", "channel_custom", "channel_name" ]);
        // chat callback
        cmd.setCallback((_cmd, _ori, out, res) => {
            // Switch
            if(res.switch){
                if(res.channel_server){
                    Chat.setPlayerChannel(_ori.player.xuid, "server");
                    return out.success(`Your chat channel is now: ${Chat.getPlayerChannel(_ori.player.xuid)}`);
                }
                else if(res.channel_sg){
                    Chat.setPlayerChannel(_ori.player.xuid, "sg");
                    return out.success(`Your chat channel is now: ${Chat.getPlayerChannel(_ori.player.xuid)}`);
                }
                else if(res.channel_custom){
                    if( res.channel_name == "server" || res.channel_name == "group" 
                     || res.channel_name == "sg" ){
                        return out.error("you can't switch to a custom channel with public name.");
                    }
                    else{
                        Chat.setPlayerChannel(_ori.player.xuid, res.channel_name);
                    }
                }
                else{
                    return out.error("unknown command.");
                }
            }
            else if(res.show){
                if(_ori.player){
                    return out.success(`Your chat channel is: ${Chat.getPlayerChannel(_ori.player.xuid)}`);
                }
                else{
                    return out.error(`The origin of this command must be player.`);
                }
            }
            else{
                return out.error("unknown command.");
            }
        });
        cmd.setup();
    },
    chatop:function(){
        let cmd = mc.newCommand("chatop", "chat commands for op.", PermType.GameMasters);
        /// getchannel
        cmd.setEnum  ("e_getchannel"       , ["getchannel"  ]);
        cmd.mandatory("getchannel"         , ParamType.Enum, "e_getchannel"  , 1 );
        /// setchannel
        cmd.setEnum  ("e_setchannel"  , ["setchannel"       ]);
        cmd.mandatory("setchannel"    , ParamType.Enum, "e_setchannel"  , 1 );
        // name
        cmd.mandatory("player_name"       , ParamType.String              );
        cmd.mandatory("channel_name"       , ParamType.String              );

        cmd.overload (["getchannel", "player_name"]);
        cmd.overload (["setchannel", "player_name", "channel_name"]);

        cmd.setCallback((_cmd, _ori, out, res) => {
            // Switch
            var xuid = data.name2xuid(res.player_name);
            if(xuid == null){
                return out.error("player not found.");
            }
            else{
                if(res.getchannel){
                    return out.success(`Player "${res.player_name}" is now in chat channel: ${Chat.getPlayerChannel(xuid)}`);
                }
                else if(res.setchannel){
                    Chat.setPlayerChannel(xuid, res.channel_name);
                }
                else{
                    return out.error("unknown command.");
                }
            }
        });
        cmd.setup();
    },
    mute:function(){
        let cmd = mc.newCommand("mute", "mute sb.", PermType.GameMasters);
        
        cmd.mandatory("player_name"   , ParamType.String );
        cmd.mandatory("time"          , ParamType.Int    );
        cmd.overload (["player_name"  , "time"          ]);

        cmd.setCallback((_cmd, _ori, out, res) => {
            // Switch
            var xuid = data.name2xuid(res.player_name);
            if(xuid == null){
                return out.error("player not found.");
            }
            else{
                Chat.setPlayerMuteTime(xuid, res.time);

                return out.success(`Mute "${res.player_name}"`);
            }
        });
        cmd.setup();
    },
    unmute:function(){
        let cmd = mc.newCommand("unmute", "unmute sb.", PermType.GameMasters);
        
        cmd.mandatory("player_name", ParamType.String );
        cmd.setCallback((_cmd, _ori, out, res) => {
            // Switch
            var xuid = data.name2xuid(res.player_name);
            if(xuid == null){
                return out.error("player not found.");
            }
            else{
                Chat.setPlayerMuteTime(xuid, 0);
                return out.success(`Unmute "${res.player_name}"`);
            }
        });
        cmd.setup();
    }
}
// ==================== RunTime =======================
///// Web Socket /////
ws.listen('onTextReceived', (msg) => {
    // logger.info("[websocket] received:" + msg);
    if (msg == null) {
        return;
    }
    else if (msg == "success") {
        logger.info("Connection established.");
        WebAPI.identityAuthentication("onServerStarted");
    }
    else {
        try {
	        var message = JSON.parse(msg);
	        if(typeof message == 'object' && message){
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
                            // WebAPI.getPlayerData(message["xuid"], "onPreJoin");
                            WebAPI.getPlayerData_HTTP(message["xuid"]);
                        }
                        // 2 - Online on this server - //End//
                        else if (message["result"] == serverName) {
                            logger.info("Player is already online on this server.");
                            WebAPI.setPlayerData(PlayerTool.getPlayerByXuid(message["xuid"]),"onPreJoin");
                            // WebAPI.setPlayerData(PlayerTool.getPlayerByXuid(message["xuid"]), "onPreJoin");
                        }
                        // 3 - Player doesn't exist - //Reply 2//
                        else if (message["result"] == "void") {
                            logger.info("Couldn\'t find player online status.");
                            var pl = PlayerTool.getPlayerByXuid(message["xuid"]);
                            WebAPI.bindGameCharacter(pl.xuid, pl.uuid, pl.realName, "onPreJoin");
                        }
                        // 4 - Online on other server or other situations - //End//
                        else {
                            PlayerCache.pushRefusedPlayer(message["xuid"], "You're already online on another server: " + message["result"]);
                            logger.info("Player is already online on server " + message["result"]);
                            // 为正常显示踢出信息， 改到进入游戏再执行
                            // PlayerTool.getPlayerByXuid(message["xuid"]).kick("You're already online on another server:" + string(message["result"]));
                        }
                    }
                    // Reply 1: Get player data - //End// 
                    // !! This is an unused reply
                    if (message["type"] == "getPlayerData") {
                        if (message["result"] != "void") {
                            var pl = PlayerTool.getPlayerByXuid(message["xuid"]);
                            PlayerTool.setFromJson(pl, message);
                            pl.refreshItems();
                        }
                    }
                    // Reply 2: Bind game character - //Reply 4//
                    if (message["type"] == "bindGameCharacter") {
                        if (message["result"] == "success") {
                            logger.info("Player profile build successfully.");
                            WebAPI.insertPlayerData(PlayerTool.getPlayerByXuid(message["xuid"]), "onPreJoin");
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
                    logger.debug("Set player online status: info: " + message["info"] + "; result: " + message["result"]);
                }
                else if (message["type"] == "getPlayerOnlineStatus") {
                    logger.debug("Player online status is: " + message["result"]);
                }
                else if (message["type"] == "setPlayerData") {
                    logger.debug("Change player cloud data: info: " + message["info"] + "; result: " + message["result"]);
                }
                else if (message["type"] == "tpw_getWorldList") {
                    var listMsg = "";
                    if (message["result"] == "success") {
                        var worlds = message["worlds"];
                        if (worlds.size() == 0) {
                            listMsg = lang["tpw.info.worldList.none"];
                        }
                        else {
                            for (var i = 0; i < worlds.size(); i++) {
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
                        var pl = PlayerTool.getPlayerByXuid(message["user"]);
                        if (pl)
                            pl.sendText(listMsg, 0);
                    }
                }
                ///// Request /////
                else if (message["type"] == "broadcastMessage") {
                    if (syn_message) {
                        var broadcastMessage = message["message"];
                        if (false && broadcastMessage["type"] == "player_die") {
                            try {
                                if (broadcastMessage["tool"] != "null") {
                                    // three parameter(player, killer, tool)
                                    mc.broadcast("{\"rawtext\":[{\"translate\":\"" + broadcastMessage["sentence"]
                                        + "\",\"with\":[\"" + broadcastMessage["player"]
                                        + "\",\"" + broadcastMessage["killer"]
                                        + "\",\"" + broadcastMessage["tool"] + "\"]}]}"
                                        , 9);
                                }
                                else if (broadcastMessage["killer"] != "null") {
                                    // two parameter(player, killer)
                                    mc.broadcast("{\"rawtext\":[{\"translate\":\"" + broadcastMessage["sentence"]
                                        + "\",\"with\":[\"" + broadcastMessage["player"]
                                        + "\",\"" + broadcastMessage["killer"] + "\"]}]}"
                                        , 9);
                                }
                                else {
                                    // one parameter(player)
                                    mc.broadcast("{\"rawtext\":[{\"translate\":\"" + broadcastMessage["sentence"]
                                        + "\",\"with\":[\"" + broadcastMessage["player"] + "\"]}]}"
                                        , 9);
                                }
                            }
                            catch(e) {
                                logger.error("Error in websocket receive message~player_die." + e);
                            }
                        }
                        else if (broadcastMessage["type"] == "player_join") {
                            try {
                                mc.broadcast("{\"rawtext\":[{\"text\":\"§e\"},{\"translate\":\"multiplayer.player.joined\",\"with\":[\"" + broadcastMessage["player"] + "\"]}]}", 9);
                            }
                            catch(e) {
                                logger.error("Error in websocket receive message~player_join." + e);
                            }
                        }
                        else if (broadcastMessage["type"] == "player_left") {
                            try {
                                mc.broadcast("{\"rawtext\":[{\"text\":\"§e\"},{\"translate\":\"multiplayer.player.left\",\"with\":[\"" + broadcastMessage["player"] + "\"]}]}", 9);
                            }
                            catch(e) {
                                logger.error("Error in websocket receive message~player_join." + e);
                            }
                        }
                        else if (broadcastMessage["type"] == "chat") {
                            Chat.sendChannelMessage(broadcastMessage["channel"], Chat.translateChatString(broadcastMessage["from"], broadcastMessage["speaker"], broadcastMessage["sentence"]));
                            logger.info(`[${broadcastMessage["from"]} ] ${broadcastMessage["speaker"]} " >> ${broadcastMessage["sentence"]}`);
                        }
                    }
                }
                else if (message["type"] == "runCommand") {
                    if (message["method"] == "direct") {
                        mc.runcmd(message["command"]);
                    }
                }
                else if (message["type"] == "tpw_setTransformingPlayer") {
                    // 本服务器作为起始服务器
                    if (message["info"] == "set") {
                        var userMsg = "";
                        // 目标服务器在线
                        if (message["result"] == "success") {
                            var successList = "";
                            var targetArray = message["targets"];
                            for (var i = 0; i < targetArray.size(); i++) {
                                var transformingPlayer = PlayerTool.getPlayerByRealName(targetArray[i]);
                                if (transformingPlayer) {
                                    transformingPlayer.sendText(lang["tpw.info.teleporting"], 0);
                                    PlayerCache.pushTransformingPlayer(transformingPlayer.realName);
                                    if (transformingPlayer.transferServer(message["IP"], message["port"])) {
                                        successList += transformingPlayer.realName;
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
                            var userPlayer = PlayerTool.getPlayerByXuid(message["user"]);
                            if(userPlayer)
                                userPlayer.sendText(userMsg, 0);
                        }
                    }
                    // 本服务器作为目标服务器
                    else if (message["info"] == "command") {
                        for (var i = 0; i < message["targets"].size(); i++) {
                            PlayerCache.pushTransformingPlayer(message["targets"][i]);
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
            else{
            logger.info("[websocket] Invalid message received: " + msg);
	        }
	    } catch(e) {
	        logger.info('error:' + msg + '\nerror code:' + e);
	    }
    }
});
ws.listen('onError', (err) => {
    logger.error("WebSocket connection error.");
    logger.error("Error message: " + err);
});
ws.listen('onLostConnection', (msg) => {
    logger.error('WebSocket connection lost. Message: ' + msg);
});
///// MC Event /////
mc.listen('onServerStarted', function() {
    // Create money scoreboard
    if(syn_moneyType == "score"){
        setTimeout(function () {
            let score = mc.getScoreObjective(syn_moneyType);
            if (score == null) {
                mc.newScoreObjective(syn_moneyType, syn_moneyType);
            }
        }, 4000);
    }
    // Connect web socket server
    ws.connect(ws_hostURL);
    CommandManager.set();
});
mc.listen('onPreJoin',(pl)=>{
    // pl.sendText("hello world~");
    // ev.mXUID;
    // ev.mIP;
    if (ws.status != 0) {
        PlayerCache.pushRefusedPlayer(pl.xuid, "Unable to reach web socket server, please contact with server administrator.");
    }
    else {
        // webAPI::setPlayerOnlineStatus(pl.xuid, "login", "onPreJoin");
        WebAPI.getPlayerOnlineStatus(pl.xuid, "onPreJoin");
    }
});
mc.listen('onJoin', (pl) => {
    PlayerDataTool.load(pl.xuid);

    var onlilneServerName = PlayerCache.findRefusedPlayer(pl.xuid);
    if (onlilneServerName == null) {
        // 玩家准备进服时修改NBT概率失败, 因此移到这里
        var newNBT = PlayerCache.getNBT(pl.xuid, true);
        if (newNBT != null) {
            PlayerTool.setFromJson(pl, newNBT);
            pl.refreshItems();
        }
        // TME会在onJoin时修改金钱, 因此这里特别延迟改一次金钱
        if (syn_money) {
            var plXuid = pl.xuid;
            setTimeout(() => {
                var pl = PlayerTool.getPlayerByXuid(plXuid);
                PlayerTool.setMoney(pl, PlayerCache.getMoney(pl, true));
            }, 500); //ms
        }
        // 完成一切同步后修改在线状态，并把玩家状态设为已同步
        WebAPI.setPlayerOnlineStatus(pl.xuid, "login", "onJoin");
        WebAPI.broadcastMessage_join(pl.realName);
        PlayerCache.pushSynchronizedPlayer(pl.xuid);
    }
    else {
        logger.info("kick " + pl.realName + " " + onlilneServerName);
        mc.runcmd("kick " + pl.realName + " " + onlilneServerName);
    }
});
mc.listen("onLeft", (pl) => {
    PlayerDataTool.release(pl.xuid);

    var refused = (PlayerCache.findRefusedPlayer(pl.xuid, true) != null);
    var synchronized = PlayerCache.findSynchronizedPlayer(pl.xuid, true);
    if (!refused && synchronized) {
        WebAPI.setPlayerData(pl, "onLeft");
        WebAPI.setPlayerOnlineStatus(pl.xuid, "logout", "onLeft");
        WebAPI.broadcastMessage_left(pl.realName);
    }
    // Delete cache for player who failed to connect.
    PlayerCache.getMoney(pl.xuid, true);
});
mc.listen('onChat', function(pl, msg) {
    if(Chat.getPlayerMuteTime(pl.xuid) == 0){
        if(msg.substring(0, 1) != ";"){
            var channel = Chat.getPlayerChannel(pl.xuid);
            // Chat.sendChannelMessage(channel, Chat.translateChatString(serverName, pl.realName, msg));
            WebAPI.broadcastMessage_chat(pl.realName, msg, channel);
        }
    }
    else{
        pl.sendText(`You are under mute now. unmute after ${Chat.getPlayerMuteTime(pl.xuid)} mins`, 0);
    }
    return true;
});
// TODO: 使用语言文件的完美转发
mc.listen('onPlayerDie', function(pl, source) {
    var name;
    if(source == undefined){
        name = "null";
    }
    else{
        name = source.name;
    }
    WebAPI.broadcastMessage_die("#%1被%2杀死了", pl.realName, name);
});
// ==================== Timer ====================
// Web Socket reconnect
setInterval(() => {
    if(ws.status == 2){
        logger.info('WebSocket connection closed!');
        logger.info('Trying reconnect WebSocket server');
        logger.info('stauts:', ws.connect(ws_hostURL), ws.status);
    }
    else if(ws.status == 0){
        ws.send("Heart beat");
    }
}, ws_timeOut * 1000);
// Auto save
setInterval(() => {
    var playerList = mc.getOnlinePlayers();
    for (var pl of playerList) {
        // 玩家没有被拒绝且背包已经同步
        var refused = (PlayerCache.findRefusedPlayer(pl.xuid) != null);
        var synchronized = PlayerCache.findSynchronizedPlayer(pl.xuid);
        if (!refused && synchronized) {
            WebAPI.setPlayerData(pl, "Auto save");
        }
    }
}, syn_autoSaveTime * 60000); //min
// Chat: mute
setInterval(() => {
    var playerList = mc.getOnlinePlayers();
    for (var pl of playerList) {
        var time = Chat.getPlayerMuteTime(pl.xuid);
        if(time > 0){
            Chat.decreasePlayerMuteTime(pl.xuid);
        }
    }
}, 60000); //min