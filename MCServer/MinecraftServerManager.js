
// ==================== Title ====================


  /* --------------------------------------- *\
   *  Name        :  MinecraftServerManager  *
   *  Description :  A server manage engine  *
   *  Version     :  alpha 0.1.0             *
   *  Author      :  ENIAC_Jushi             *
  \* --------------------------------------- */


// ================== Initialize ==================

// log output
logger.setConsole(true);
logger.setTitle('MSM');
logger.info('MinecraftServerManager is running');

var WS = new WSClient();

// floder
if(!file.exists("plugins/MinecraftServerManager")) {
    file.mkdir("plugins/MinecraftServerManager");
}

// config
if(!file.exists("plugins/MinecraftServerManager/config.json")) {
    file.writeTo("plugins/MinecraftServerManager/config.json", JSON.stringify({
        "hostURL": 'localhost:8080',
        "serverName": 'Server_1',
        "key": "S@H12=S-NA",
        "timeOut": 15,
        "autoSaveTime": 5,
        "synNBT": true,
        "synScores": true,
        "synTags": true,
        "synLLMoney": true,
        "synMessage": true,
        "debug": false
    } , null , '\t'));
    setTimeout(() => { logger.info('未找到配置文件，以默认配置启动'); }, 6000);
}
Config = JSON.parse(file.readFrom("plugins/MinecraftServerManager/config.json"));

// Debug mode
var debug = Config["debug"];
// Web socket config
var hostURL = Config["hostURL"];
var WSHostURL = "ws://"+ hostURL +"/ws/asset"
var key = Config["key"];
var serverName = Config["serverName"];
// Time
var timeOut = Config["timeOut"];
var autoSaveTime = Config["autoSaveTime"];
// Synchronize config
var synNBT = Config["synNBT"];
var synScores = Config["synScores"];
var synTags = Config["synTags"];
var synLLMoney = Config["synLLMoney"];
var synMessage = Config["synMessage"];

// ===================== API =====================
// Identity authentication
function identityAuthentication(info = "default"){
    WS.send(JSON.stringify({
        type: 'identityAuthentication',
        authentication: 'server',
        key: key,
        name: serverName,
        info: info
    }));
}

// Bind game character
function bindGameCharacter(xuid, uuid, realName, info = "default"){
    WS.send(JSON.stringify({
        type: 'bindGameCharacter',
        xuid: xuid,
        uuid: uuid,
        realName: realName,
        info: info
    }));
}

// Set player online status
function setPlayerOnlineStatus(xuid, operate, info = "default"){
    WS.send(JSON.stringify({
        type: 'setPlayerOnlineStatus',
        xuid: xuid,
        operate: operate,
        info: info
    }));
}

// Set player data, "undo" to not set
function setPlayerData(pl, info = "default"){
    var message = {
        type: 'setPlayerData',
        xuid: pl.xuid,
        info: info
    }
    message.NBT     = synNBT     ? getPlayerData_NBT_local(pl).toSNBT()           : "false";
    message.scores  = synScores  ? JSON.stringify(getPlayerData_scores_local(pl)) : "false";
    message.LLMoney = synLLMoney ? money.get(pl.xuid)                             : "false";
    message.tags    = synTags    ? JSON.stringify(pl.getAllTags())                : "false";

    WS.send(JSON.stringify(message));
}
// Insert player Data
function insertPlayerData(pl, info = "default"){
    var message = {
        type: 'insertPlayerData',
        xuid: pl.xuid,
        info: info
    }
    message.NBT     = synNBT     ? getPlayerData_NBT_local(pl).toSNBT()           : "false";
    message.scores  = synScores  ? JSON.stringify(getPlayerData_scores_local(pl)) : "false";
    message.LLMoney = synLLMoney ? money.get(pl.xuid)                             : "false";
    message.tags    = synTags    ? JSON.stringify(pl.getAllTags())                : "false";
    WS.send(JSON.stringify(message));
}
// Get player Data
function getPlayerData(xuid, info = "default"){
    WS.send(JSON.stringify({
        type: 'getPlayerData',
        info: info,
        xuid: xuid,
        NBT: synNBT,
        scores: synScores,
        LLMoney: synLLMoney,
        tags: synTags
    }));
}
// Get player Data
// 使用websocket传输会报错，可能是因为玩家数据很长
function getPlayerData_HTML(xuid, callback){
    var requestURL = "http://" + hostURL + "/getPlayerData?xuid=" + xuid
        + (synNBT     ? "&NBT=true"     : "&NBT=false"    )
        + (synScores  ? "&scores=true"  : "&scores=false" )
        + (synTags    ? "&tags=true"    : "&tags=false"   )
        + (synLLMoney ? "&LLMoney=true" : "&LLMoney=false");
    network.httpGet(requestURL, (status, result) => {
        if(status == 200){
            var resultJson = toJSON(result);
            var pl = mc.getPlayer(resultJson.xuid);
            setPlayerData_local(pl, resultJson);
            pl.refreshItems();
        }
    });
}

// ===================== Tools =====================
function toJSON(str) {
    if (typeof str == 'string') {
        try {
            var obj = JSON.parse(str);
            if(typeof obj == 'object' && obj ){return obj;} 
            else {return false;}
        } catch(e) {}
    }
}
function debugLog(log){
    if(debug) logger.info(log);
}
// =================== Functions ===================
function getPlayerData_NBT_local(pl){
    var playerNBT = pl.getNbt();
    var playerNBTData = new NbtCompound({
        'Inventory': playerNBT.getTag('Inventory'),
        'EnderChest': playerNBT.getTag('EnderChestInventory'),
        'Offhand': playerNBT.getTag('Offhand'),
        'Armor': playerNBT.getTag('Armor'),
        'Attributes': playerNBT.getTag('Attributes'),
        'PlayerLevel': playerNBT.getTag('PlayerLevel'),
        'PlayerLevelProgress': playerNBT.getTag('PlayerLevelProgress')
    });
    playerNBT.destroy();
    return playerNBTData;
}
function getPlayerData_scores_local(pl){
    var scoreboard = mc.getAllScoreObjectives();
    
    var scores = {};
    scores.score = {};
    for(var i in scoreboard) {
        var objectives = scoreboard[i].name;
        try{
            scores.score[objectives] = pl.getScore(objectives);
        }catch(e){}
    }
    return scores;
}
function getPlayerData_local(pl){
    if(pl.getExtraData('NBTERROR') != 'ERROR') {
        var playerNBT = pl.getNbt();
        var playerNBTData = new NbtCompound({
            'Inventory': playerNBT.getTag('Inventory'),
            'EnderChest': playerNBT.getTag('EnderChestInventory'),
            'Offhand': playerNBT.getTag('Offhand'),
            'Armor': playerNBT.getTag('Armor'),
            'Attributes': playerNBT.getTag('Attributes'),
            'PlayerLevel': playerNBT.getTag('PlayerLevel'),
            'PlayerLevelProgress': playerNBT.getTag('PlayerLevelProgress')
        });

        var scoreboard = mc.getAllScoreObjectives();
        var Data_ = {};
        
        Data_.score = {};
        for(var i in scoreboard) {
            var objectives = scoreboard[i].name;
            try{
                Data_.score[objectives] = pl.getScore(objectives);
            }catch(e){}
        }

        var data_ = {
            NBT: playerNBTData.toSNBT(),
            LLMoney: money.get(pl.xuid),
            scoreboard: Data_,
            tags: pl.getAllTags()
        }
        playerNBT.destroy();
        playerNBTData.destroy();
        return data_;
    }
    else{
        return "error";
    }
}
function setPlayerData_local(pl, message){
    if((message != undefined) && (message != false)) {
        // NBT
        if(synNBT){
            var NBT_ = NBT.parseSNBT(message.NBT);
            var playerNBT = pl.getNbt();
            // Bag
            playerNBT.setTag("Inventory",  NBT_.getTag("Inventory" ));
            playerNBT.setTag("Armor",      NBT_.getTag("Armor"     ));
            playerNBT.setTag("Offhand",    NBT_.getTag("Offhand"   ));
            playerNBT.setTag("Attributes", NBT_.getTag("Attributes"));
            // Level
            playerNBT.setTag("PlayerLevel", NBT_.getTag("PlayerLevel"));
            playerNBT.setTag("PlayerLevelProgress", NBT_.getTag("PlayerLevelProgress"));
            // Ender chest
            playerNBT.setTag("EnderChestInventory", NBT_.getTag("EnderChest"));

            if(pl.setNbt(playerNBT)) {
                pl.refreshItems();
                debugLog(`Success: Local NBT data of ${pl.realName} has been changed.`);
            }
            else {
                debugLog(`Failed: Local NBT data of ${pl.realName} has not been changed`);
                pl.setExtraData('NBTERROR', 'ERROR');
                pl.disconnect(`${system.getTimeStr()} [WSClinet-Syn] NBT set failed.`);
            }
        }

        // Tag
        if(synTags){
            if(message.tags != [] ) {
                for(var i in message.tags) {
                    pl.addTag(message.tags[i]);
                }
                debugLog(`Success: Local tag data of ${pl.realName} has been changed.`);
            }
        }

        // Scoreboard
        if(synScores){
            var score_ = message.scores;
            if(score_.score != []) {
                var objectives = score_.score;
                for(var i in objectives) {
                    if (mc.getScoreObjective(i) == undefined) {
                        mc.newScoreObjective(i, i)
                    }
                    pl.setScore(i, objectives[i]);
                }
                debugLog(`Success: Local scoreboard data of ${pl.realName} has been changed.`);
            }
        }

        // LLMoney
        if(synLLMoney){
            money.set(pl.xuid, parseInt(message.LLMoney));
            debugLog(`Success: Local LLMoney data of ${pl.realName} has been changed.`);
        }
        
        // end
        NBT_.destroy();
        playerNBT.destroy();
    }
}
// =================== WS Events ===================
// On text received
WS.listen('onTextReceived', (msg) => {
    if(msg != undefined && msg == "success"){
        debugLog("Connection established.");
        identityAuthentication("onServerStarted");
    }
    else{
        var message = toJSON(msg);
        // Reply: Identity authentication - onServerStarted
        if(message != undefined && message.type == 'identityAuthentication' && message.info == 'onServerStarted') {
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
                    getPlayerData_HTML(message.xuid);
                    // getPlayerData(message.xuid, "onPreJoin");
                }
                // 2 - Online on this server - //End//
                else if(message.result == serverName){
                    setPlayerData(mc.getPlayer(message.xuid), "onPreJoin");
                    debugLog('Player is already online on this server.');
                }
                // 3 - Player doesn't exist - //Reply 2//
                else if(message.result == "void"){
                    pl = mc.getPlayer(message.xuid);
                    bindGameCharacter(pl.xuid, pl.uuid, pl.realName, "onPreJoin");
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
                    setPlayerData_local(pl, message);
                    pl.refreshItems();
                }
            }
            // Reply 2: Bind game character - //Reply 4//
            if(message != undefined && message.type == 'bindGameCharacter'){
                if(message.result == "success"){
                    insertPlayerData(mc.getPlayer(message.xuid), "onPreJoin");
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
                if(message.message.type == "chat"){
                    mc.broadcast('[' + Format.Aqua + broadcastMessage.from + Format.Clear + '] ' +
                                 Format.MinecoinGold + broadcastMessage.speaker + Format.Clear + ' >> ' + broadcastMessage.sentence);
                    mc.sendCmdOutput('[' + broadcastMessage.from + '] ' + broadcastMessage.speaker + ' >> ' + broadcastMessage.sentence);
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
    logger.info('Trying connect WebSocket server...');
    if(WS.status != 0) {
        debugLog('stauts:', WS.connect(WSHostURL), WS.status);

    }
    else{
        logger.info('Connection established.');
    }
});

mc.listen('onPreJoin',(pl)=>{
    if(WS.status == 0){
        // set player online status to login
        setPlayerOnlineStatus(pl.xuid, "login", "onPreJoin");
    }
    else{
        pl.disconnect("Unable to reach web socket server, please contact the administrator.");
    }
});

mc.listen('onJoin', (pl) => {

});

mc.listen("onLeft", (pl) => {
    setPlayerOnlineStatus(pl.xuid, "logout", "onLeft");
    setPlayerData(pl, "onLeft");
});

mc.listen('onChat',function(pl,msg) {
    if(synMessage) {
        WS.send(JSON.stringify({
            type: 'broadcastMessage',
            message:{
                type: "chat",
                from: serverName,
                speaker: pl.realName,
                sentence: msg
            }
        }));
    }
});

// ==================== Timer ====================
// ws reconnect
setInterval(() => {
    if(WS.status == 2){
        debugLog('WebSocket connection closed!');
        debugLog('Trying reconnect to WebSocket server');
        debugLog('stauts:', WS.connect(WSHostURL), WS.status);

    }
    else if(WS.status == 0){
        WS.send("Heart beat");
    }
}, 1000 * timeOut);

// Auto save
setInterval(() => {
    var playerList = mc.getOnlinePlayers();
    if(JSON.stringify(playerList) != '[]') {
        for(var i in playerList) {
            setPlayerData(playerList[i]), "Auto save";
        }
    }
}, 60000 * autoSaveTime);