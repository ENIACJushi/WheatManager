
// ==================== Title ====================


  /* --------------------------------------- *\
   *  Name        :  MinecraftServerManager  *
   *  Description :  A server manage engine  *
   *  Version     :  Alpha 0.1.0             *
   *  Author      :  ENIAC_Jushi             *
  \* --------------------------------------- */


// ================== Initialize ==================
let version = 0.1
// log output
logger.setConsole(true);
logger.setTitle('MSM');
logger.info('MinecraftServerManager is running');

// Global variable
var WS = new WSClient();

// Cache
var Cache = {
    tag           : {} , // xuid : tags
    money         : {} , // xuid : money
    refusedPlayer : []   // xuid, xuid...
};

// floder
if(!file.exists("plugins/MinecraftServerManager")) {
    file.mkdir("plugins/MinecraftServerManager");
}

// config
if(!file.exists("plugins/MinecraftServerManager/config.json")) {
    file.writeTo("plugins/MinecraftServerManager/config.json", JSON.stringify({
        "debug"            : false            ,
        "WebSocket"        :                  {
            "hostURL"      : "localhost:8080" ,
            "serverName"   : "CERN_httpd"     ,
            "key"          : "S@H12=S-NA"     ,
            "timeOut"      : 15               ,
        }                                     ,
        "Synchronous"      :                  {
            "autoSaveTime" : 5                ,
            "NBT"          : true             ,
            "scores"       : true             ,
            "tags"         : true             ,
            "message"      : true             ,
            "money"        :                  {
                type       : "score"          ,
                name       : "money"          ,
                enable     : true
            }
        }
    } , null , '\t'));
    setTimeout(() => { logger.info('未找到配置文件，以默认配置启动'); }, 6000);
}
Config = JSON.parse(file.readFrom("plugins/MinecraftServerManager/config.json"));

// Debug mode
const debug        = Config[ "debug"       ];
// Web socket config
const hostURL      = Config[ "WebSocket"   ][ "hostURL"      ];
const key          = Config[ "WebSocket"   ][ "key"          ];
const serverName   = Config[ "WebSocket"   ][ "serverName"   ];
const timeOut      = Config[ "WebSocket"   ][ "timeOut"      ] * 1000;
const WSHostURL    = "ws://" + hostURL + "/ws/asset";
// Synchronize config
const autoSaveTime = Config[ "Synchronous" ][ "autoSaveTime" ] * 60000;
const synNBT       = Config[ "Synchronous" ][ "NBT"          ];
const synScores    = Config[ "Synchronous" ][ "scores"       ];
const synTags      = Config[ "Synchronous" ][ "tags"         ];
const synMessage   = Config[ "Synchronous" ][ "message"      ];
const synMoney     = Config[ "Synchronous" ][ "money"        ][ "enable" ];
const synMoneyType = Config[ "Synchronous" ][ "money"        ][ "type"   ];
const synMoneyName = Config[ "Synchronous" ][ "money"        ][ "name"   ];

// Create money scoreboard
if(synMoneyType == "score"){
    setTimeout(function () {
        let score = mc.getScoreObjective(synMoneyName);
        if (score == null) {
            mc.newScoreObjective(synMoneyName, synMoneyName);
        }
    }, 4000);
}
// ===================== API =====================
// Class: Web Socket API
const WSAPI = {
    identityAuthentication : function(info = "default"){
        WS.send(JSON.stringify({
            type           : 'identityAuthentication',
            authentication : 'server'                ,
            key            : key                     ,
            name           : serverName              ,
            info           : info
        }));
    },
    bindGameCharacter      : function(xuid, uuid, realName, info = "default"){
        WS.send(JSON.stringify({
            type           : 'bindGameCharacter'     ,
            xuid           : xuid                    ,
            uuid           : uuid                    ,
            realName       : realName                ,
            info           : info
        }));
    },
    setPlayerOnlineStatus  : function(xuid, operate, info = "default"){
        WS.send(JSON.stringify({
            type           : 'setPlayerOnlineStatus' ,
            xuid           : xuid                    ,
            operate        : operate                 ,
            info           : info
        }));
    },
    setPlayerData          : function(pl, info = "default"){    // "false" to not set
        var message = {
            type           : 'setPlayerData'         ,
            xuid           : pl.xuid                 ,
            info           : info
        }
        message.NBT        = synNBT    ? PDM.getPlayerData_NBT(pl).toSNBT()           : "false" ;
        message.scores     = synScores ? JSON.stringify(PDM.getPlayerData_scores(pl)) : "false" ;
        message.money      = synMoney  ? PDM.getPlayerData_money(pl)                  : "false" ;
        message.tags       = synTags   ? JSON.stringify(pl.getAllTags())              : "false" ;
    
        WS.send(JSON.stringify(message));
    },
    insertPlayerData       : function(pl, info = "default"){
        var message = {
            type           : 'insertPlayerData'      ,
            xuid           : pl.xuid                 ,
            info           : info
        }
        message.NBT        = synNBT    ? PDM.getPlayerData_NBT(pl).toSNBT()           : "false" ;
        message.scores     = synScores ? JSON.stringify(PDM.getPlayerData_scores(pl)) : "false" ;
        message.money      = synMoney  ? PDM.getPlayerData_money(pl)                  : "false" ;
        message.tags       = synTags   ? JSON.stringify(pl.getAllTags())              : "false" ;
        WS.send(JSON.stringify(message));
    },
    // TODO: 无法使用的API：不支持op code 0，可能是因为玩家数据很长
    getPlayerData          : function(xuid, info = "default"){
        WS.send(JSON.stringify({
            type           : 'getPlayerData'         ,
            info           : info                    ,
            xuid           : xuid                    ,
            NBT            : synNBT                  ,
            scores         : synScores               ,
            money          : synMoney                ,
            tags           : synTags
        }));
    },
    // TODO: 使用HTML的替代方法，因为只是读取，所以没有特别添加权限验证
    getPlayerData_HTML     : function(xuid, callback){
    var requestURL         = "http://" + hostURL + "/getPlayerData?xuid=" + xuid
        + ( synNBT         ? "&NBT=true"     : "&NBT=false"     )
        + ( synScores      ? "&scores=true"  : "&scores=false"  )
        + ( synTags        ? "&tags=true"    : "&tags=false"    )
        + ( synMoney       ? "&money=true"   : "&money=false"   );
    network.httpGet(requestURL, (status, result) => {
        if(status == 200){
            var resultJson = toJSON(result);
            var pl         = mc.getPlayer(resultJson.xuid);
            setPlayerDataFromMessage(pl, resultJson);
            pl.refreshItems();
        }
    });
    }
}

// =================== Functions ===================
// Class: Player Data Manager
const PDM = {
    // Get local player data
    getPlayerData_NBT:function(pl){
        var playerNBT = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'Inventory'           : playerNBT.getTag( 'Inventory'           ),
            'Offhand'             : playerNBT.getTag( 'Offhand'             ),
            'Armor'               : playerNBT.getTag( 'Armor'               ),

            'EnderChest'          : playerNBT.getTag( 'EnderChestInventory' ),

            'Attributes'          : playerNBT.getTag( 'Attributes'          ),

            'PlayerLevel'         : playerNBT.getTag( 'PlayerLevel'         ),
            'PlayerLevelProgress' : playerNBT.getTag( 'PlayerLevelProgress' )
        });
        playerNBT.destroy();
        return playerNBTData;
    },
    getPlayerData_scores:function(pl){
        var scoreboard = mc.getAllScoreObjectives();
        var scores = {};
        for(var i in scoreboard) {
            scores[scoreboard[i].name] = pl.getScore(scoreboard[i].name);
        }
        return scores;
    },
    getPlayerData_money:function(pl){
        if(synMoneyType == "score"){
            return pl.getScore(synMoneyName);
        }
        else if(synMoneyType == "LLMoney"){
            return money.get(pl.xuid);
        }
        else{
            logger.error("Failed to get player data: money.\n  Invalid money type.(\'score\' or \'LLMoney\')")
        }
    },
    // Set local player data
    setPlayerData_NBT:function(pl, NBT    ){
        if(NBT == null){
            debugLog(`Set player data successfully: ${pl.realName}, NBT, not set.`);
        }
        else{
            var playerNBT = pl.getNbt();
            playerNBT.setTag( "Inventory"           , NBT.getTag( "Inventory"           ));
            playerNBT.setTag( "Armor"               , NBT.getTag( "Armor"               ));
            playerNBT.setTag( "Offhand"             , NBT.getTag( "Offhand"             ));
            playerNBT.setTag( "Attributes"          , NBT.getTag( "Attributes"          ));
            playerNBT.setTag( "PlayerLevel"         , NBT.getTag( "PlayerLevel"         ));
            playerNBT.setTag( "PlayerLevelProgress" , NBT.getTag( "PlayerLevelProgress" ));
            playerNBT.setTag( "EnderChestInventory" , NBT.getTag( "EnderChest"          ));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                debugLog(`Set player data successfully: ${pl.realName}, NBT.`);
            }
            else {
                logger.error(`Failed to set player data: ${pl.realName}, NBT`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [MSM] NBT set failed.`);
            }
            NBT.destroy();
            playerNBT.destroy();
        }
    },
    setPlayerData_scores:function(pl, scores ){
        if(scores == undefined){
            debugLog(`Set player data successfully: ${pl.realName}, scoreboard, not set.`);
        }
        else{
            for(var i in scores) {
                if (mc.getScoreObjective(i) == undefined) {
                    mc.newScoreObjective(i, i)
                }
                pl.setScore(i, scores[i]);
            }
            debugLog(`Set player data successfully: ${pl.realName}, scoreboard.`);
        }
    },
    setPlayerData_money:function(pl, money  ){
        if(money == undefined){
            debugLog(`Set player data successfully: ${pl.realName}, money, not set.`);
        }
        else{
            if(synMoneyType == "score"){
                pl.setScore(synMoneyName, money);
                debugLog(`Set player data successfully: ${pl.realName}, money, ${money}.`);
            }
            else if(synMoneyType == "LLMoney"){
                money.set(pl.xuid, money);
                debugLog(`Set player data successfully: ${pl.realName}, money, ${money}.`);
            }
            else{
                logger.error("Failed to set player data: money.\n  Invalid money type.(\'score\' or \'LLMoney\')")
            }
        }
    },
    setPlayerData_tags:function(pl, tagArray){
        if(tagArray == undefined){
            debugLog(`Set player data successfully: ${pl.realName}, no tags.`);
        }
        else{
            var allTags = pl.getAllTags();
            for(tag in allTags){
                pl.removeTag(allTags[tag]);
            }
            for(tag in tagArray) {
                pl.addTag(tagArray[tag]);
            }
            debugLog(`Set player data successfully: ${pl.realName}, tags.`);
        }
    }
}

/**
  * Set player data from a message.
  * Tag and Money will execute until 'onjoin'.
  */
function setPlayerDataFromMessage  (pl, message){
    if((message != undefined) && (message != false)) {
        if(synTags)   Cache.tag  [pl.xuid] = toJSON(message.tags);
        if(synMoney)  Cache.money[pl.xuid] = message.money;
        if(synScores) PDM.setPlayerData_scores(pl, toJSON(message.scores)    );
        if(synNBT)    PDM.setPlayerData_NBT   (pl, NBT.parseSNBT(message.NBT));
    }
}

/**
  * Judge if the player has been denied a connection.
  * Set 'del' to true to delete it from cache list.
  */
function isRefusedPlayer      (xuid, del = false){
    for(xuid in Cache.refusedPlayer){
        if(Cache.refusedPlayer[xuid] == xuid){
            if(del) delete Cache.refusedPlayer[xuid]
            return true;
        }
    }
    return false;
}

// ===================== Tools =====================
function toJSON(str) {
    if (typeof str == 'string') {
        try {
            var obj = JSON.parse(str);
            if(typeof obj == 'object' && obj ) return obj; 
            else return false;
        } catch(e) {}
    }
}
function debugLog(log){
    if(debug) logger.info(log);
}

// =================== WS Events ===================
// On text received
WS.listen('onTextReceived', (msg) => {
    if(msg != undefined && msg == "success"){
        debugLog("Connection established.");
        WSAPI.identityAuthentication("onServerStarted");
    }
    else{
        var message = toJSON(msg);
        // Reply: Identity authentication - onServerStarted
        if(message != undefined && message.type == 'identityAuthentication' 
        && message.info == 'onServerStarted') {
            if(message.result == "success"){
                debugLog('Authentication passed.');
            }
        }
        // Reply: onPreJoin
        if(message != undefined && message.info == 'onPreJoin'){
            // Reply 0: Set player online status
            if(message.type == 'setPlayerOnlineStatus') {
                // 1 - Offline - //Reply 1//
                if(message.result == "success"){
                    debugLog('Set player online status to ' + serverName);
                    WSAPI.getPlayerData_HTML(message.xuid);
                    // getPlayerData(message.xuid, "onPreJoin");
                }
                // 2 - Online on this server - //End//
                else if(message.result == serverName){
                    WSAPI.setPlayerData(mc.getPlayer(message.xuid), "onPreJoin");
                    debugLog('Player is already online on this server.');
                }
                // 3 - Player doesn't exist - //Reply 2//
                else if(message.result == "void"){
                    pl = mc.getPlayer(message.xuid);
                    WSAPI.bindGameCharacter(pl.xuid, pl.uuid, pl.realName, "onPreJoin");
                    debugLog('Couldn\'t find player online status.');
                }
                // 4 - Online on other server or other situations - //End//
                else{
                    mc.getPlayer(message.xuid).disconnect("You're already online on another server:" + message.result);
                    debugLog('Player is already online on server ' + message.result);
                }
            }
            // Reply 1: Get player data - //End// 
            // !! This is an unused reply
            if(message != undefined && message.type == 'getPlayerData'){
                if(message.result != "void"){
                    var pl = mc.getPlayer(message.xuid);
                    setPlayerDataFromMessage(pl, message);
                    pl.refreshItems();
                }
            }
            // Reply 2: Bind game character - //Reply 4//
            if(message != undefined && message.type == 'bindGameCharacter'){
                if(message.result == "success"){
                    WSAPI.insertPlayerData(mc.getPlayer(message.xuid), "onPreJoin");
                }
            }
            // Reply 3: Set player Data - //End//
            if(message != undefined && message.type == 'setPlayerData'){
                if(message.result == "success"){
                    debugLog('Success: Cloud data of player has been changed');
                }
                else{
                    debugLog('Falied: Cloud data of player has not been changed');
                }
            }
            // Reply 4: Insert player Data
            if(message != undefined && message.type == 'insertPlayerData'){
                if(message.result == "success"){
                    debugLog('Success: Cloud data of player has been insert');
                }
                else{
                    debugLog('Falied: Cloud data of player has not been insert');
                }
            }
        }
        // Request: broadcastMessage
        if(message != undefined && message.type == 'broadcastMessage'){
            if(synMessage) {
                var broadcastMessage = message.message;
                // Chat message
                if(broadcastMessage.type == "chat"){
                    mc.broadcast('[' + Format.Aqua +  broadcastMessage.from  + Format.Clear + '] ' +
                                 Format.MinecoinGold +  broadcastMessage.speaker + Format.Clear  + ' >> ' + broadcastMessage.sentence);
                    mc.sendCmdOutput('[' + broadcastMessage.from + '] ' + broadcastMessage.speaker + ' >> ' + broadcastMessage.sentence);
                }
                // Death message
                else if(broadcastMessage.type == "player_die"){
                    if(broadcastMessage.name == "null"){
                        mc.broadcast(broadcastMessage.player + " 失败了", 0);
                    }
                    else{
                        mc.broadcast(broadcastMessage.player + " 被 " + broadcastMessage.source + " 打败了", 0);
                    }
                }
            }
        }
    }
});
// Error
WS.listen('onError', (err) => {
    debugLog('WebSocket connection error.');
    logger.error('Error code: ' + err);
});

// Lost connection
WS.listen('onLostConnection', (msg) => {
    logger.error('WebSocket connection lost. Error code: ' + msg);
});

// =================== MC Events ===================
mc.listen('onServerStarted', function() {
    // 因为在一个版本，开服时连接会造成服务器假死，暂时取消。
    if(false){
        logger.info('Trying connect WebSocket server...');
        if(WS.status != 0) {
            debugLog('stauts:', WS.connect(WSHostURL), WS.status);
        }
        else{
            logger.info('Connection established.');
        }
    }

});

mc.listen('onPreJoin',(pl)=>{
    if(WS.status == 0){
        // set player online status to login
        WSAPI.setPlayerOnlineStatus(pl.xuid, "login", "onPreJoin");
    }
    else{
        pl.disconnect("Unable to reach web socket server, please contact the administrator.");
    }
});

mc.listen('onJoin', (pl) => {
    // 在某次修bug的时候挪了过来，当时认为是prejoin没有标签，但并没有验证到底需不需要
    if(synTags){
        if(Cache.tag[pl.xuid] != undefined) {
            PDM.setPlayerData_tags(pl, Cache.tag[pl.xuid]);
        }
        delete Cache.tag[pl.xuid];
    }
    // TME会在onJoin时修改金钱，因此这里特别延迟改一次金钱
    if(synMoney){
        setTimeout(() => {
            PDM.setPlayerData_money(pl, Cache.money[pl.xuid]);
            delete Cache.money[pl.xuid];
        }, 500);
    }
});

mc.listen("onLeft", (pl) => {
    if(!isRefusedPlayer(pl.xuid), true){
        WSAPI.setPlayerOnlineStatus(pl.xuid, "logout", "onLeft");
        WSAPI.setPlayerData(pl, "onLeft");
    }
    // Delete cache for player who failed to connect
    delete Cache.tag  [pl.xuid];
    delete Cache.money[pl.xuid];
});

mc.listen('onChat', function(pl,msg) {
    if(synMessage) {
        WS.send(JSON.stringify({
            type: 'broadcastMessage',
            message:{
                type     : "chat"      ,
                from     : serverName  ,
                speaker  : pl.realName ,
                sentence : msg
            }
        }));
    }
});

mc.listen('onPlayerDie', function(pl, source) {
    var name;
    if(source == undefined){
        name = "null";
    }
    else{
        name = source.name;
    }
    if(synMessage) {
        WS.send(JSON.stringify({
            type: 'broadcastMessage',
            message:{
                type     : "player_die",
                from     : serverName  ,
                player   : pl.realName ,
                source   : name
            }
        }));
    }
});

// ==================== Timer ====================
// ws reconnect
setInterval(() => {
    if(WS.status == 2){
        debugLog('WebSocket connection closed!');
        debugLog('Trying reconnect WebSocket server');
        debugLog('stauts:', WS.connect(WSHostURL), WS.status);
    }
    else if(WS.status == 0){
        WS.send("Heart beat");
    }
}, timeOut);

// Auto save
setInterval(() => {
    debugLog('Auto save');
    var playerList = mc.getOnlinePlayers();
    if(JSON.stringify(playerList) != '[]') {
        for(var i in playerList) {
            if(!isRefusedPlayer(playerList[i].xuid)){
                WSAPI.setPlayerData(playerList[i]), "Auto save";
            }
        }
    }
}, autoSaveTime);