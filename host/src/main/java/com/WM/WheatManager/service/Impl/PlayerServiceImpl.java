package com.WM.WheatManager.service.Impl;

import com.WM.WheatManager.dao.PlayerDao;
import com.WM.WheatManager.service.PlayerService;
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
        return (result == null) ? "void" : result;
    }
    @Override
    // 设置指定玩家的在线状态
    public String setPlayerOnlineStatus(String xuid, String status) {
        if(playerDao.setPlayerOnlineStatus(xuid, status)){
            return "success";
        }
        return "failed";
    }
    @Override
    // 获得指定玩家的在线时长
    public Integer queryPlayerOnlineTime(String xuid){
        Integer result = playerDao.queryPlayerOnlineTime(xuid);
        if(result == null){
            return -1;
        }
        else{
            return result;
        }
    }
    @Override
    // 增加在线玩家的在线时长
    public void addOnlinePlayerOnlineTime(int time){
        playerDao.addOnlinePlayerOnlineTime(time);
    }
    @Override
    // 增加指定玩家的在线时长
    public void addPlayerOnlineTime(String xuid, int timeIncrement){
        playerDao.addPlayerOnlineTime(xuid, timeIncrement);
    };
    @Override
    public String getPlayerNameByXuid(String xuid){
        String result = playerDao.getPlayerNameByXuid(xuid);
        return (result == null) ? "unknown" : result;
    }
}
