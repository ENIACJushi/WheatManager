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
public interface PlayerDataDao {
    // 获得指定玩家的Data
    String queryPlayerData(String xuid);
    // 设置指定玩家的Data
    boolean setPlayerData(String xuid, String data);
    // 添加一个新的玩家Data
    boolean insertPlayerData(String xuid, String data);
}
