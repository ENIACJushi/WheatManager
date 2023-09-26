package com.WM.WheatManager.dao;

public interface QQDao {
    // 创建记录
    boolean insertData(String name, long number);
    // 删除记录
    boolean removeData(long number);
    // 设置QQ号
    boolean setNumber(String name, long number);
    // 设置玩家名
    boolean setName(long number, String name);
    // 获取玩家名对应的qq号
    Integer getNumber(String name);
    // 获取qq号对应的玩家名
    String getName(long number);
}

