// MSM_G001 ☆ 验证连接属性 ~ IdentityAuthentication
// MSM_S001 ☆ Data同步 ~ SynchronousData
// MSM_S003 ☆ 设置玩家在线状态 ~ SynchronousData
package com.WM.WheatManager.controller;

import com.WM.WheatManager.entity.WebSocketClient;
import com.WM.WheatManager.entity.OnlinePlayerCache;
import com.WM.WheatManager.service.PlayerDataService;
import com.WM.WheatManager.service.PlayerService;

import com.WM.WheatManager.service.QQService;
import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.context.annotation.Bean;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.socket.server.standard.ServletServerContainerFactoryBean;

import javax.annotation.PostConstruct;
import javax.websocket.*;
import javax.websocket.server.ServerEndpoint;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.atomic.AtomicInteger;

@Component
@RestController
@ServerEndpoint(value = "/ws/asset")
public class WebSocketController {
    @Bean
    public ServletServerContainerFactoryBean createWebSocketContainer() {
        ServletServerContainerFactoryBean container = new ServletServerContainerFactoryBean();
        // 在此处设置bufferSize
        container.setMaxTextMessageBufferSize(512000);
        container.setMaxBinaryMessageBufferSize(512000);
        container.setMaxSessionIdleTimeout(15 * 60000L);
        return container;
    }
    private static PlayerDataService playerDataService;
    private static PlayerService playerService;
    private static QQService qqService;
    public static String serverKey;
    public static String botKey;
    @Autowired
    public void setConfig(PlayerDataService service) {
        playerDataService = service;
    }
    @Autowired
    public void setConfig(PlayerService service) {
        playerService = service;
    }
    @Autowired
    public void setConfig(QQService service) {
        qqService = service;
    }
    @Value("${spring.socket.serverKey}")
    public void setConfig(String key){serverKey = key;}
    @Value("${spring.socket.botKey}")
    public void setConfigBotKey(String bKey){botKey = bKey;}
//    @Autowired
//    private PlayerDataService playerDataService;
//    @Autowired
//    private PlayerService playerService;

    // 连接信息
    @PostConstruct
    public void init() {
        log.info("[WebSocket] Initialize..");
    }
    private static final Logger log = LoggerFactory.getLogger(WebSocketController.class);
    private static final AtomicInteger OnlineCount = new AtomicInteger(0);
    private static final CopyOnWriteArraySet<WebSocketClient> ClientSet = new CopyOnWriteArraySet<WebSocketClient>();
    // 在线玩家信息
    public static OnlinePlayerCache onlinePlayerCache = new OnlinePlayerCache();

    // 连接建立成功
    @OnOpen
    public void onOpen(Session session) {
        ClientSet.add(new WebSocketClient(session));
        int cnt = OnlineCount.incrementAndGet(); // 在线数加1
        log.info("[WebSocket] New connection, present connection amount：{},sessionId={}", cnt,session.getId());
        SendMessage(session, "success");
    }

    // 连接关闭
    @OnClose
    public void onClose(Session session) {

        ClientSet.removeIf(c -> c.session.getId().equals(session.getId()));

        int cnt = OnlineCount.decrementAndGet();
        log.info("[WebSocket] 有连接关闭，当前连接数为：{}", cnt);
    }

    // 收到客户端消息
    @OnMessage
    public void onMessage(String message, Session session) {
        // 日志
        if(message.equals("Heart beat")) return;
        log.info("[WebSocket] 来自客户端的消息：{}",message);
        // 解析json
        JSONObject jsonMessage = JSON.parseObject(message);
        // 确定请求类型
        String type = jsonMessage.getString("type");
        // 确定发起者角色
        WebSocketClient client = null;
        for(WebSocketClient c : ClientSet){
            if(c.session == session){
                client = c;
                break;
            }
        }
        if (client == null) {
            SendMessage(session,"No session");
            return;
        }
        // 初始化回复信息
        JSONObject reply = new JSONObject();
        reply.put("info", jsonMessage.getString("info"));
        reply.put("type", jsonMessage.getString("type"));
        // 处理请求并答复
        switch (type) {
            // 验证连接属性
            case "identityAuthentication":
                // 服务器
                if (jsonMessage.getString("authentication").equals("server")) {
                    if (jsonMessage.getString("key").equals(serverKey)) {
                        for (WebSocketClient c : ClientSet) {
                            if (c.session.getId().equals(session.getId())) {
                                c.setName(jsonMessage.getString("name"));
                                c.setType("server");
                                c.IP = jsonMessage.getString("IP");
                                c.port = jsonMessage.getInteger("port");
                                // success
                                log.info("[WebSocket] A new server is verified.");
                                reply.put("result", "success");
                                SendMessage(session, reply.toJSONString()); return;
                            }
                        }
                    }
                }
                // Q群机器人
                else if (jsonMessage.getString("authentication").equals("bot")) {
                    log.info("bot");
                    if (jsonMessage.getString("key").equals(botKey)) {
                        for (WebSocketClient c : ClientSet) {
                            if (c.session.getId().equals(session.getId())) {
                                c.setName(jsonMessage.getString("name"));
                                c.setType("bot");
                                // success
                                log.info("[WebSocket] A new bot is verified.");
                                reply.put("result", "success");
                                SendMessage(session, reply.toJSONString()); return;
                            }
                        }
                    }
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            // ---------------------- 信息同步 ----------------------//
            case "broadcastMessage":
                // 转换
                if(client.type.equals("botFutrue")){
                    jsonMessage.put("sender", qqService.getName(jsonMessage.getLong("sender")));
                }
                if(client.type.equals("server") || client.type.equals("bot")){
                    // 转换放这里


                    Session s = null;
                    for (WebSocketClient c : ClientSet) {
                        // 发送给类型为server或bot，且不是信息来源的连接
                        if(c.type.equals("server")||c.type.equals("bot")) {
                            if (c.session.isOpen() && !c.name.equals(client.name)) {
                                s = c.session;
                                SendMessage(s, message);
                            }
                        }
                    }
                }
                return;
            case "pushCommand":
                if(client.type.equals("server") || client.type.equals("bot")){
                    // 准备指令
                    JSONObject commandJson = new JSONObject();
                    commandJson.put("type"   , "runCommand");
                    commandJson.put("info"   , jsonMessage.getString("info")   );
                    commandJson.put("method" , jsonMessage.getString("method") );
                    commandJson.put("command", jsonMessage.getString("command"));
                    String commandMsg = commandJson.toString();
                    // 发送指令
                    String target = jsonMessage.getString("target");
                    int successAmount = 0;
                    if(target.equals("ALL")){
                        for (WebSocketClient c : ClientSet) {
                            if(c.type.equals("server")) {
                                SendMessage(c.session, commandMsg);
                                successAmount++;
                            }
                        }
                    }
                    else{
                        for (WebSocketClient c : ClientSet) {
                            if(c.type.equals("server") && c.name.equals(target)) {
                                SendMessage(c.session, commandMsg);
                                successAmount++;
                            }
                        }
                    }
                    // 回应
                    reply.put("result"       , "success"                            );
                    reply.put("info"         , jsonMessage.getString("info"   ));
                    reply.put("sender"       , jsonMessage.getString("sender" ));
                    reply.put("command"      , jsonMessage.getString("command"));
                    reply.put("successAmount", successAmount                        );
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            // -------------------- 操作玩家数据 --------------------//
            //
            // 绑定玩家游戏角色
            case "bindGameCharacter":
                reply.put("xuid", jsonMessage.getString("xuid"));
                if(client.type.equals("server")){
                    reply.put("result", playerService.bindGameCharacter(jsonMessage.getString("xuid"),
                            jsonMessage.getString("uuid"), jsonMessage.getString("realName")));
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
                // 查询玩家在线状态
            case "getPlayerOnlineStatus":
                if(client.type.equals("server")){
                    // 查询在线状态
                    String xuid = jsonMessage.getString("xuid");
                    reply.put("xuid", xuid);
                    reply.put("result", playerService.queryPlayerOnlineStatus(xuid));
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            // 设置玩家在线状态
            case "setPlayerOnlineStatus":
                if(client.type.equals("server")){
                    // 查询在线状态
                    String xuid = jsonMessage.getString("xuid");
                    reply.put("xuid", xuid);

                    String onlineStatus = playerService.queryPlayerOnlineStatus(xuid);
                    // 执行操作，当没有在线记录时返回void
                    // 登入 仅在当前状态为下线时设置，返回success，否则返回当前在线状态
                    if (jsonMessage.getString("operate").equals("login")) {
                        if (onlineStatus.equals("offline")) {
                            playerService.setPlayerOnlineStatus(xuid, client.name);
                            // 加入在线玩家缓存
                            onlinePlayerCache.setPlayerOnline(xuid,System.currentTimeMillis() / 60000);
                            reply.put("result", "success");
                        }
                        else {
                            reply.put("result", onlineStatus);
                        }
                    }
                    // 登出 仅在当前状态为在发起请求的服务器在线时设置，返回offline，否则返回当前在线状态
                    else if (jsonMessage.getString("operate").equals("logout")) {
                        if (onlineStatus.equals(client.name)) {
                            playerService.setPlayerOnlineStatus(xuid, "offline");
                            // 移出在线玩家缓存并增加在线时间
                            onlinePlayerCache.setPlayerOffline(xuid,System.currentTimeMillis() / 60000);
                            reply.put("result", "offline");
                        }
                        else {
                            reply.put("result", onlineStatus);
                        }
                        log.info("Player logout.");
                    }
                    // 仅查询 返回当前在线状态
                    else {
                        reply.put("result", onlineStatus);
                    }
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;

            // 新增玩家数据
            case "insertPlayerData":
                if(client.type.equals("server")){
                    String xuid = jsonMessage.getString("xuid");
                    reply.put("xuid", xuid);

                    String bag        = jsonMessage.getString("bag");
                    String enderChest = jsonMessage.getString("enderChest");
                    String attributes = jsonMessage.getString("attributes");
                    String level      = jsonMessage.getString("level");
                    String scores     = jsonMessage.getString("scores");
                    String tags       = jsonMessage.getString("tags");
                    Integer money     = jsonMessage.getInteger("money");
                    reply.put("result", playerDataService.insertPlayerData
                            (xuid, bag, enderChest, attributes, level, scores, tags, money));
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;

            // 获取玩家数据
            case "getPlayerData":
                if(client.type.equals("server")){
                    String xuid = jsonMessage.getString("xuid");
                    reply.put("xuid", xuid);

                    boolean bag = jsonMessage.getBoolean("bag");
                    boolean enderChest = jsonMessage.getBoolean("enderChest");
                    boolean attributes = jsonMessage.getBoolean("attributes");
                    boolean level = jsonMessage.getBoolean("level");
                    boolean scores = jsonMessage.getBoolean("scores");
                    boolean tags = jsonMessage.getBoolean("tags");
                    boolean money = jsonMessage.getBoolean("money");

                    List<Map<String, Object>> data = playerDataService.queryPlayerData
                            (xuid, bag, enderChest, attributes, level, scores, tags, money);
                    if (data == null || data.isEmpty()) {
                        reply.put("result", "void");
                    }
                    else {
                        reply.put("result", "success");
                        reply.put("bag",        data.get(0).get("bag"));
                        reply.put("enderChest", data.get(0).get("ender_chest"));
                        reply.put("attributes", data.get(0).get("attributes"));
                        reply.put("level",      data.get(0).get("level"));
                        reply.put("scores",     data.get(0).get("scores"));
                        reply.put("tags",       data.get(0).get("tags"));
                        reply.put("money",      data.get(0).get("money"));
                    }
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;

            // 设置玩家数据
            case "setPlayerData":
                if(client.type.equals("server")){
                    String xuid = jsonMessage.getString("xuid");
                    reply.put("xuid", xuid);

                    String bag        = jsonMessage.getString("bag");
                    String enderChest = jsonMessage.getString("enderChest");
                    String attributes = jsonMessage.getString("attributes");
                    String level      = jsonMessage.getString("level");
                    String scores     = jsonMessage.getString("scores");
                    String tags       = jsonMessage.getString("tags");
                    Integer money     = jsonMessage.getInteger("money");
                    log.info("money:" + jsonMessage.getInteger("money"));
                    reply.put("result", playerDataService.setPlayerData
                            (xuid, bag, enderChest, attributes, level, scores, tags, money));
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            // 跨服传送中继
            case "tpw_setTransformingPlayer":
                if(client.type.equals("server")){
                    reply.put("user", jsonMessage.getString("user"));
                    reply.put("targets", jsonMessage.getJSONArray("targets"));
                    reply.put("destination", jsonMessage.getString("destination"));

                    WebSocketClient desClient = getClientByName(jsonMessage.getString("destination"));
                    if(desClient == null){
                        reply.put("result", "failed");
                    }
                    else{
                        reply.put("result", "success");
                        reply.put("targets", jsonMessage.getJSONArray("targets"));
                        reply.put("IP", desClient.IP);
                        reply.put("port", desClient.port);
                        // 让目标服务器做好准备
                        JSONObject desMessage = new JSONObject();
                        desMessage.put("type", "tpw_setTransformingPlayer");
                        desMessage.put("info", "command");
                        desMessage.put("targets", jsonMessage.getJSONArray("targets"));
                        SendMessage(desClient.session, desMessage.toJSONString());
                    }
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            case "tpw_getWorldList":
                reply.put("users", jsonMessage.getString("users"));
                if(client.type.equals("server")){
                    reply.put("result", "success");
                    reply.put("user", jsonMessage.getString("user"));
                    JSONArray worldList = new JSONArray();
                    for (WebSocketClient c : ClientSet) {
                        if(!c.name.equals(client.name)) worldList.add(c.name);
                    }
                    reply.put("worlds", worldList);
                    SendMessage(session, reply.toJSONString()); return;
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            // -------------------- QQBot操作 --------------------//
            case "whiteList":
                if(client.type.equals("bot")){
                    reply.put("number", jsonMessage.getLong("number"));
                    reply.put("name", jsonMessage.getString("name"));
                    reply.put("method", jsonMessage.getString("method"));
                    reply.put("sender", jsonMessage.getString("sender"));
                    boolean process;
                    // 增添
                    if(jsonMessage.getString("method").equals("insert")){
                        /// 绑定QQ号 ///
                        if(qqService.insertData(jsonMessage.getString("name"), jsonMessage.getLong("number")).equals("success")){
                            /// 添加白名单 ///
                            // 准备白名单指令
                            JSONObject whiteCommand = new JSONObject();
                            String playerName = jsonMessage.getString("name");
                            if(playerName.contains(" "))
                                playerName = "\"" + playerName + "\"";
                            whiteCommand.put("type"  , "runCommand");
                            whiteCommand.put("info"  , "default"   );
                            whiteCommand.put("method", "direct"    );
                            whiteCommand.put("command", "whitelist add " + playerName);
                            String commandMsg = whiteCommand.toString();
                            // 向所有在线服务器发送白名单指令
                            for (WebSocketClient c : ClientSet) {
                                if(c.type.equals("server")) {
                                    SendMessage(c.session, commandMsg);
                                }
                            }

                            /// 回复 ///
                            reply.put("result", "success");
                            SendMessage(session, reply.toJSONString()); return;
                        }
                    }
                    // 修改
                    else if(jsonMessage.getString("method").equals("change")){
                        // 获取旧玩家名
                        String oldName = qqService.getName(jsonMessage.getLong("number"));
                        if(oldName.equals("null")){
                            /// 回复 ///
                            reply.put("result", "failed");
                            reply.put("info", "not_exist");
                            SendMessage(session, reply.toJSONString()); return;
                        }
                        /// 修改玩家名 ///
                        if(qqService.setName(jsonMessage.getLong("number"), jsonMessage.getString("name")).equals("success")){
                            /// 修改白名单 ///
                            // 准备添加白名单指令
                            JSONObject whiteCommand = new JSONObject();
                            String playerName = jsonMessage.getString("name");
                            if(playerName.contains(" "))
                                playerName = "\"" + playerName + "\"";
                            whiteCommand.put("type"  , "runCommand");
                            whiteCommand.put("info"  , "default"   );
                            whiteCommand.put("method", "direct"    );
                            whiteCommand.put("command", "whitelist add " + playerName);
                            String commandMsg = whiteCommand.toString();

                            // 准备删除白名单指令
                            JSONObject removeCommand = new JSONObject();
                            if(oldName.contains(" "))
                                oldName = "\"" + oldName + "\"";
                            removeCommand.put("type"  , "runCommand");
                            removeCommand.put("info"  , "default"   );
                            removeCommand.put("method", "direct"    );
                            removeCommand.put("command", "whitelist remove " + oldName);
                            String removeCommandMsg = removeCommand.toString();

                            // 向所有在线服务器发送白名单指令
                            for (WebSocketClient c : ClientSet) {
                                if(c.type.equals("server")) {
                                    SendMessage(c.session, removeCommandMsg);
                                    SendMessage(c.session, commandMsg);
                                }
                            }
                            /// 回复 ///
                            reply.put("result", "success");
                            SendMessage(session, reply.toJSONString()); return;
                        }
                    }
                    // 删除
                    else if(jsonMessage.getString("method").equals("delete")){
                        String oldName = qqService.getName(jsonMessage.getLong("number"));
                        if(qqService.removeData(jsonMessage.getLong("number")).equals("success")){
                            // 准备删除白名单指令
                            JSONObject removeCommand = new JSONObject();
                            if(oldName.contains(" "))
                                oldName = "\"" + oldName + "\"";
                            removeCommand.put("type"  , "runCommand");
                            removeCommand.put("info"  , "default"   );
                            removeCommand.put("method", "direct"    );
                            removeCommand.put("command", "whitelist remove " + oldName);
                            String removeCommandMsg = removeCommand.toString();
                            // 向所有在线服务器发送白名单指令
                            for (WebSocketClient c : ClientSet) {
                                if(c.type.equals("server")) {
                                    SendMessage(c.session, removeCommandMsg);
                                }
                            }
                            reply.put("name", oldName);
                            reply.put("result", "success");
                            SendMessage(session, reply.toJSONString()); return;
                        }
                    }
                }
                reply.put("result", "failed");
                SendMessage(session, reply.toJSONString()); return;
            default:
                reply.put("result", "Invalid request");
                SendMessage(session, reply.toJSONString());
        }
    }

    // 出现错误
    @OnError
    public void onError(Session session, Throwable error) {
        log.error("发生错误：{}，Session ID： {}",error.getMessage(),session.getId());
        error.printStackTrace();
    }
    ///// 功能接口 /////
    // 向QQ群机器人发送json信息
    public static void sendMessageToBot(String message){
        for (WebSocketClient c : ClientSet) {
            // 发送给类型为bot的连接
            if (c.session.isOpen() && c.type.equals("bot")) {
                Session s = c.session;
                SendMessage(s, message);
            }
        }
    }
    public static void sendSentenceToBot(String message){
        JSONObject botMessage       = new JSONObject();
        JSONObject msg       = new JSONObject();

        botMessage.put("type","broadcastMessage");

        msg.put("type"         , "sentence"     );
        msg.put("sentence", message);

        botMessage.put("message"      , msg    );
        sendMessageToBot(botMessage.toJSONString());
    }
    ///// 基础接口 /////
    // 发送消息，实践表明，每次浏览器刷新，session会发生变化。
    public static void SendMessage(Session session, String message) {
        try {
            // session.getBasicRemote().sendText(String.format("%s (From Server，Session ID=%s)",message,session.getId()));
            log.info("[WebSocket] Send message");
            session.getBasicRemote().sendText(message);
        } catch (IOException e) {
            log.error("发送消息出错：{}", e.getMessage());
            e.printStackTrace();
        }
    }
    // 群发消息
    public static void BroadCastInfo(String message) throws IOException {
        for (WebSocketClient client : ClientSet) {
            if(client.session.isOpen()){
                SendMessage(client.session, message);
            }
        }
    }
    // 指定Session发送消息
    public static void SendMessage(String message,String sessionId) throws IOException {
        Session session = null;
        for (WebSocketClient client : ClientSet) {
            if(client.session.getId().equals(sessionId)){
                session = client.session;
                break;
            }
        }
        if(session!=null){
            SendMessage(session, message);
        }
        else{
            log.warn("没有找到你指定ID的会话：{}",sessionId);
        }
    }
    // 获得指定名称服务器的连接
    public static Session getSessionByName(String name){
        Session session = null;
        for (WebSocketClient client : ClientSet) {
            if(client.name.equals(name)){
                session = client.session;
                break;
            }
        }
        return session;
    }

    public static WebSocketClient getClientByName(String name){
        WebSocketClient client = null;
        for (WebSocketClient c : ClientSet) {
            if(c.name.equals(name)){
                client = c;
                break;
            }
        }
        return client;
    }
}