package com.WM.WheatManager.entity;

import com.WM.WheatManager.service.PlayerService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.annotation.RestController;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

@Component
@RestController
public class OnlinePlayerCache {

    public static PlayerService playerService;
    @Autowired
    public void setConfig(PlayerService service) {
        playerService = service;
    }

    public List<OnlinePlayerMessage> onlinePlayerList;

    public OnlinePlayerCache(){
        onlinePlayerList = new ArrayList<>();
    }
    ///// 玩家 /////
    // 将玩家设为在线，已存在时返回false，不存在时新建并返回true
    public boolean setPlayerOnline(String xuid, long enterTime){
        for(OnlinePlayerMessage onlinePlayerMessage : onlinePlayerList){
            if(onlinePlayerMessage.xuid.equals(xuid)){
                onlinePlayerMessage.setEnterTime(enterTime);
                onlinePlayerMessage.status = true;
                return false;
            }
        }
        OnlinePlayerMessage onlinePlayerMessage = new OnlinePlayerMessage(xuid, 0, enterTime, true);
        onlinePlayerList.add(onlinePlayerMessage);
        return true;
    }
    // 将玩家设为离线，已存在时设置并返回时间增量，不存在时返回0
    public long setPlayerOffline(String xuid, long nowTime) {
        for (OnlinePlayerMessage onlinePlayerMessage : onlinePlayerList) {
            if (onlinePlayerMessage.xuid.equals(xuid)) {
                long timeIncrement = nowTime - onlinePlayerMessage.enterTime;
                onlinePlayerMessage.onlineTime += timeIncrement;
                onlinePlayerMessage.status = false;
                if(timeIncrement != 0)
                    playerService.addPlayerOnlineTime(xuid, (int) timeIncrement);
                return timeIncrement;
            }
        }
        return 0;
    }
    // 更新玩家在线时间
    public long refreshPlayerOnlineTime(String xuid, long nowTime){
        for(OnlinePlayerMessage onlinePlayerMessage : onlinePlayerList){
            if(onlinePlayerMessage.xuid.equals(xuid)){
                int timeIncrement = onlinePlayerMessage.refreshOnlineTime(nowTime);
                if(timeIncrement != 0)
                    playerService.addPlayerOnlineTime(xuid, timeIncrement);
                return timeIncrement;
            }
        }
        return 0;
    }
    // 获取指定xuid的玩家的在线信息
    public OnlinePlayerMessage getPlayerMessage(String xuid){
        for(OnlinePlayerMessage onlinePlayerMessage : onlinePlayerList){
            if(onlinePlayerMessage.xuid.equals(xuid)){
                return onlinePlayerMessage;
            }
        }
        return null;
    }
    public void test(){
        playerService.setPlayerOnlineStatus("2535468723693953", "SC");
    }
}
