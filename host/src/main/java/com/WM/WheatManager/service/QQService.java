package com.WM.WheatManager.service;

public interface QQService {
    // 创建记录
    String insertData(String name, long number);
    // 删除记录
    String removeData(long number);
    // 设置QQ号
    String setNumber(String name, long number);
    // 设置玩家名
    String setName(long number, String name);
    // 获取玩家名对应的qq号
    long getNumber(String name);
    // 获取qq号对应的玩家名
    String getName(long number);
}
