package com.WM.WheatManager.entity;

public class OnlinePlayerMessage {
    public String xuid;
    public int    onlineTime;
    public long   enterTime;
    public boolean status;

    public OnlinePlayerMessage(String xuid, int onlineTime, long enterTime, boolean status) {
        this.xuid       = xuid ;
        this.onlineTime = onlineTime;
        this.enterTime  = enterTime;
        this.status     = status;
    }
    public void setXuid(String xuid){
        this.xuid       = xuid ;
    }
    public void setOnlineTime(int onlineTime){
        this.onlineTime = onlineTime ;
        this.status = true;
    }
    public void setEnterTime(long enterTime){
        this.enterTime  = enterTime ;
        this.status = false;
    }
    public int refreshOnlineTime(long nowTime){
        long timeIncrement = nowTime - enterTime;
        onlineTime += timeIncrement;
        enterTime = nowTime;
        return (int)timeIncrement;
    }
    public void resetOnlineTime(){
        onlineTime = 0;
    }
}
