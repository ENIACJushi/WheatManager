package com.MSM.MinecraftServerManager.service;

import java.util.List;
import java.util.Map;

/**
 * <p>
 * Description : TODO
 * </p>
 *
 * @author : ENIAC_Jushi
 * @version : 1.0
 * @date : 2022.03
 */
public interface PlayerDataService {
    // 获得指定玩家的Data
    List<Map<String, Object>> queryPlayerData(String xuid, boolean NBT, boolean scores, boolean tags, boolean money);
    // 设置指定玩家的Data
    String setPlayerData(String xuid, String NBT,String scores,String tags,Integer money);
    // 添加一个玩家Data
    public String insertPlayerData(String xuid, String NBT,String scores,String tags,Integer money);
}
