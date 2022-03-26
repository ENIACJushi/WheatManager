package com.MSM.MinecraftServerManager.service.Impl;

import com.MSM.MinecraftServerManager.dao.PlayerDao;
import com.MSM.MinecraftServerManager.service.PlayerService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * <p>
 * Description : TODO
 * </p>
 *
 * @author : ENIAC_Jushi
 * @version : 1.0
 * @date : 2022.03
 */
@Service
public class PlayerServiceImpl implements PlayerService {
    @Autowired
    PlayerDao playerDao;

    @Override
    // 新增一条玩家记录
    public String bindGameCharacter(String xuid, String uuid,String realName){
        if(playerDao.bindGameCharacter(xuid, uuid, realName)){
            return "success";
        }
        else{
            if(playerDao.queryPlayerOnlineStatus(uuid) != null){
                return "failed(existed)";
            }
        }
        return "failed";
    }
    @Override
    // 获得指定玩家的在线状态
    public String queryPlayerOnlineStatus(String xuid){
        String result = playerDao.queryPlayerOnlineStatus(xuid);
        if(result == null){
            return "void";
        }
        else{
            return result;
        }
    }
    @Override
    // 设置指定玩家的在线状态
    public String setPlayerOnlineStatus(String xuid, String status) {
        if(playerDao.setPlayerOnlineStatus(xuid, status)){
            return "success";
        }
        return "failed";
    }
}
