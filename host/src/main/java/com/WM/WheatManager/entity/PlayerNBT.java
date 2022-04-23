package com.WM.WheatManager.entity;


public class PlayerNBT {
    private String uid ;
    private String data;
    private String time;

    public PlayerNBT() { }

    public PlayerNBT(String uid, String data, String time) {
        this.uid  = uid ;
        this.data = data;
        this.time = time;
    }

    @Override
    public String toString() {
        return "PlayerBag{" + "uid=" + uid + ", data=" + data + ", time=" + time + '}';
    }

    public String getUid ()            { return uid;       }
    public void   setUid (String uid)  { this.uid = uid;   }

    public String getData()            { return data;      }
    public void   setData(String data) { this.data = data; }

    public String getTime()            { return time;      }
    public void   setTime(String time) { this.time = time; }

}

