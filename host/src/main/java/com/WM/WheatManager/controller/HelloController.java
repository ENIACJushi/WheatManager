package com.WM.WheatManager.controller;

import com.WM.WheatManager.dao.QQDao;
import com.WM.WheatManager.entity.OnlinePlayerCache;
import com.WM.WheatManager.controller.WebSocketController;
import com.WM.WheatManager.entity.OnlinePlayerMessage;
import com.WM.WheatManager.service.PlayerService;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.Iterator;
import com.WM.WheatManager.task.task;

@RestController
public class HelloController {
    @Autowired
    QQDao qqDao;
    @Autowired
    PlayerService playerService;

    // 测试用接口
    @RequestMapping("/hello")
    public String hello(){
        OnlinePlayerCache onlinePlayerCache = new OnlinePlayerCache();
        onlinePlayerCache.test();

        return "ok";
    }
    @RequestMapping("/qqmessage")
    public String qqMessage(@RequestParam("msg")String msg){
        WebSocketController.sendSentenceToBot(msg);

        return "ok";
    }
    @RequestMapping("/refresh")
    public String refresh(){
        JSONArray  onlinePlayerList = new JSONArray();
        JSONObject botMessage       = new JSONObject();
        Iterator<OnlinePlayerMessage> iterator = WebSocketController.onlinePlayerCache.onlinePlayerList.iterator();

        int allOnlineTime = 0;
        int playerAmount  = 0;
        while(iterator.hasNext()){
            OnlinePlayerMessage playerOnlineMessage = iterator.next();
            // 玩家在线则当场计算在线时间
            if(playerOnlineMessage.status){
                playerOnlineMessage.refreshOnlineTime(System.currentTimeMillis() / 60000);
            }

            // 统计
            JSONObject onlinePlayerMessage = new JSONObject();
            onlinePlayerMessage.put("xuid", playerOnlineMessage.xuid);
            onlinePlayerMessage.put("onlineTime", playerOnlineMessage.onlineTime);
            onlinePlayerMessage.put("name", playerService.getPlayerNameByXuid(playerOnlineMessage.xuid));
            allOnlineTime += playerOnlineMessage.onlineTime;
            playerAmount ++;
            // onlinePlayerMessage.put(playerOnlineMessage.xuid, playerOnlineMessage.onlineTime);
            onlinePlayerList.add(onlinePlayerMessage);

            // 玩家在线则重置记录
            if(playerOnlineMessage.status){
                playerOnlineMessage.resetOnlineTime();
            }
            // 玩家不在线则清除记录
            else{
                iterator.remove();
            }
        }

        botMessage.put("type"         , "statistics"     );
        botMessage.put("info"         , "onlineMessage"  );
        botMessage.put("allOnlineTime", allOnlineTime    );
        botMessage.put("playerAmount" , playerAmount     );
        botMessage.put("playerList"   , onlinePlayerList );

        WebSocketController.sendMessageToBot(botMessage.toJSONString());
        return "ok";
    }
}