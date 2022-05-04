package com.WM.WheatManager.entity;

import javax.websocket.Session;

public class WebSocketClient {
    public Session session;
    public String type;
    public String name;
    public String IP;
    public int port;

    public WebSocketClient() { }
    public WebSocketClient(Session session){
        this.session = session;
    }
    public void setType(String type){
        this.type = type;
    }
    public void setName(String name){
        this.name = name;
    }
}
