package com.WM.WheatManager.service;

/**
 * <p>
 * Description : TODO
 * </p>
 *
 * @author : ENIAC_Jushi
 * @version : 1.0
 * @date : 2022.03
 */
public interface PlayerService {
    // 新增一条玩家记录
    String bindGameCharacter(String xuid, String uuid,String realName);
    // 获得指定玩家的在线状态
    String queryPlayerOnlineStatus(String xuid);
    // 设置指定玩家的在线状态
    String setPlayerOnlineStatus(String xuid, String status);
}
