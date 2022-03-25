package com.MSM.MinecraftServerManager.dao;

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
}
