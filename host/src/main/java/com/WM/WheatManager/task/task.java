package com.WM.WheatManager.task;

import com.WM.WheatManager.controller.WebSocketController;

import com.WM.WheatManager.entity.OnlinePlayerMessage;
import com.WM.WheatManager.service.PlayerService;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.Iterator;

@Component
@EnableScheduling
//@Async
public class task {
    @Autowired
    PlayerService playerService;
    // 每周一上午8点
    @Scheduled(cron = "0 0 8 ? * MON")
    public void Hello1() throws InterruptedException {
        WebSocketController.sendSentenceToBot("新的一周! ");
        WebSocketController.sendSentenceToBot("以崭新的面貌迎接本周的工作或学习吧 ^ ^ ");
    }
    // 每周二上午8点
    @Scheduled(cron = "0 0 8 ? * 2")
    public void Hello2() throws InterruptedException {
        WebSocketController.sendSentenceToBot("早");
    }
    // 每周三上午8点
    @Scheduled(cron = "0 0 8 ? * 3")
    public void Hello3() throws InterruptedException {
        WebSocketController.sendSentenceToBot("人生没有白费的努力，也没有碰巧的成功");
        WebSocketController.sendSentenceToBot("我们至今为止积累的东西都不是无用的");
        WebSocketController.sendSentenceToBot("今后只要我们不停下来，道路就会不断延伸......");
    }
    // 每周四上午8点
    @Scheduled(cron = "0 0 17 ? * 4")
    public void Hello4() throws InterruptedException {
        WebSocketController.sendSentenceToBot("V我50");
    }
    // 每周五晚上10点
    @Scheduled(cron = "0 0 22 ? * 5")
    public void Hello5() throws InterruptedException {
        WebSocketController.sendSentenceToBot("晚");
    }
    // 每周六晚上7点
    @Scheduled(cron = "0 0 19 ? * 6")
    public void Hello6() throws InterruptedException {
        WebSocketController.sendSentenceToBot("周六是时候放松一下了");
        WebSocketController.sendSentenceToBot("保持规律作息，不要熬太晚，提前晚安 ^ ^");
    }
    // 每周日10点统计上线人数
    @Scheduled(cron = "0 0 22 ? * 7")
    public void refreshPlayerOnlineTime() throws InterruptedException {
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
    }
}