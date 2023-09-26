package org.DogeLake.webSocket;

import java.net.URI;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import org.DogeLake.Plugin;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.enums.ReadyState;
import org.java_websocket.handshake.ServerHandshake;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *  websocket客户端监听类
 * @author 。
 */

public class MyWebSocketClient extends WebSocketClient {
    public static final String ws_key = "aksjdfcoiuaie";
    private static Logger logger = LoggerFactory.getLogger(MyWebSocketClient.class);

    public MyWebSocketClient(URI serverUri) {
        super(serverUri);
    }

    @Override
    public void onOpen(ServerHandshake serverHandshake) {
        // 连接成功，向web socket服务端发送验证信息
        JSONObject object = new JSONObject();
        object.put("type", "identityAuthentication");
        object.put("info", "onConnect");
        object.put("authentication", "bot");
        object.put("key", ws_key);
        object.put("name", "qqBot");
        send(object.toJSONString());
        logger.info(">>>>>>>>>>>websocket open");
    }

    @Override
    // 收到信息
    public void onMessage(String message) {
        Plugin.logger.info(message);
        JSONObject jsonMessage = JSON.parseObject(message);
        switch (jsonMessage.getString("type")){
            case "identityAuthentication":
                logger.info("Authentication passed.");
                break;
            case "broadcastMessage":
                JSONObject broadcastMessage = jsonMessage.getJSONObject("message");
                if(broadcastMessage.getString("type").equals("chat")){
                    if(Plugin.channel.equals("sg") && broadcastMessage.getString("channel").equals("sg")){
                        String msg = broadcastMessage.getString("speaker") + ">\n "
                                + broadcastMessage.getString("sentence");
                        Plugin.sendMessage(688524595, msg);
                    }
                }
                else if(broadcastMessage.getString("type").equals("sentence")){
                    String msg = broadcastMessage.getString("sentence");
                    Plugin.sendMessage(688524595, msg);
                }
                break;
            case "statistics":
                if(jsonMessage.getString("info").equals("onlineMessage")){
                    StringBuilder msg = new StringBuilder("服务器运行总结\n 上线人数:");
                    msg.append(jsonMessage.getString("playerAmount")).append("\n 在线总时长:");
                    int time = jsonMessage.getInteger("allOnlineTime");
                    if(time >= 60){
                        int time_hour = time/60;
                        msg.append(time_hour).append("小时");
                    }
                    int time_minute = time % 60;
                    if(time_minute > 0){
                        msg.append(time_minute).append("分钟");
                    }
                    JSONArray playerList = jsonMessage.getJSONArray("playerList");
                    if(playerList != null){
                        msg.append("\n名单:");
                        for(int i = 0; i<playerList.size(); i++){
                            msg.append("\n").append(playerList.getJSONObject(i).getString("name"));
                        }
                    }

                    Plugin.sendMessage(830025085, msg.toString());
                    Plugin.sendMessage(688524595, msg.toString());
                }
                break;
            case "whiteList":
                if(jsonMessage.getString("result").equals("success")){
                    long sender = Long.parseLong(jsonMessage.getString("sender"));
                    if(jsonMessage.getString("result").equals("success")){
                        switch (jsonMessage.getString("method")){
                            case "insert":
                                Plugin.sendMessage(sender,jsonMessage.getLong("number") + "的白名单已添加^ ^\n 游戏名是\"" +
                                        jsonMessage.getString("name") + "\"");
                                break;
                            case "change":
                                Plugin.sendMessage(sender,jsonMessage.getLong("number") + "的白名单已修改^ ^\n 现在的游戏名是\"" +
                                        jsonMessage.getString("name") + "\"");
                                break;
                            case "delete":
                                Plugin.sendMessage(sender,jsonMessage.getLong("number") + "的白名单已删除^ ^\n 游戏名是\"" +
                                        jsonMessage.getString("name") + "\"");
                                break;
                        }
                    }
                    else{
                        Plugin.sendMessage(sender,jsonMessage.getLong("number") + "的白名单没有被改变：D");
                    }
                }
                break;
            case "pushCommand":
                if(jsonMessage.getString("result").equals("success")){
                    Plugin.sendMessage(Long.parseLong(jsonMessage.getString("sender")),
                            "执行请求已发送到" + jsonMessage.getString("successAmount") + "个服务器");
                }
                else{
                    Plugin.sendMessage(Long.parseLong(jsonMessage.getString("sender")),
                            "执行失败");
                }
                break;
            default: break;
        }
        logger.info(">>>>>>>>>> websocket message");
    }

    @Override
    public void onClose(int i, String s, boolean b) {
        logger.info(">>>>>>>>>>>websocket close");
    }

    @Override
    public void onError(Exception err) {
        logger.error(">>>>>>>>>websocket error {}",err);
    }
    /// 接口 ///
    // 广播信息
    public void broadcastMessage(String type, String speaker, String sentence, String channel){
        JSONObject jsonMsg = new JSONObject();
        jsonMsg.put("type", "broadcastMessage");
        jsonMsg.put("info", "bot");

        JSONObject message = new JSONObject();

        message.put("type", type);
        message.put("from", "QQ");
        message.put("speaker", speaker);
        message.put("sentence", sentence);
        message.put("channel", channel);

        jsonMsg.put("message", message);

        send(jsonMsg.toJSONString());
    }
    // 添/改/删 白名单
    public void whiteList(long groupId, String name, long number, String method){
        JSONObject jsonMsg = new JSONObject();
        jsonMsg.put("type"  , "whiteList");
        jsonMsg.put("info"  , "bot");
        jsonMsg.put("sender", String.valueOf(groupId));
        jsonMsg.put("name"  , name);
        jsonMsg.put("number", number);
        jsonMsg.put("method", method);

        send(jsonMsg.toJSONString());
    }
    // 执行指令
    public void pushCommand(long groupId, String target, String command, String method){
        JSONObject jsonMsg = new JSONObject();
        jsonMsg.put("type"    , "pushCommand"          );
        jsonMsg.put("info"    , "bot"                  );
        jsonMsg.put("sender"  , String.valueOf(groupId));
        // 指令信息
        jsonMsg.put("target"  , target);
        jsonMsg.put("method"  , method);
        jsonMsg.put("command" , command);

        send(jsonMsg.toJSONString());
    }
}