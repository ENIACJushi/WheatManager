package com.WM.WheatManager.dao;

/**
 * <p>
 * Description : TODO
 * </p>
 *
 * @author : ENIAC_Jushi
 * @version : 1.0
 * @date : 2022.03
 */
public interface PlayerDao {
    // 新增一条玩家记录
    boolean bindGameCharacter(String xuid, String uuid,String realName);
    // 获得指定玩家的在线状态
    String queryPlayerOnlineStatus(String xuid);
    // 设置指定玩家的在线状态
    boolean setPlayerOnlineStatus(String xuid, String status);
    // 获得指定玩家的在线时长
    Integer queryPlayerOnlineTime(String xuid);
    // 增加在线玩家的在线时长
    void addOnlinePlayerOnlineTime(int time);
    // 增加指定玩家的在线时长
    void addPlayerOnlineTime(String xuid, int timeIncrement);
    // 由xuid获取玩家的name
    String getPlayerNameByXuid(String xuid);
}
