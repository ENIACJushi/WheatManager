package com.MSM.MinecraftServerManager.service.Impl;

import com.MSM.MinecraftServerManager.dao.PlayerDataDao;
import com.MSM.MinecraftServerManager.service.PlayerDataService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.jdbc.core.JdbcTemplate;
import org.springframework.stereotype.Service;

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
@Service
public class PlayerDataServiceImpl implements PlayerDataService {
    @Autowired
    private PlayerDataDao playerDataDao;
    @Autowired
    JdbcTemplate jdbcTemplate;

    @Override
    // 获得指定玩家的Data
    public List<Map<String, Object>> queryPlayerData(String xuid, boolean NBT, boolean scores, boolean tags, boolean LLMoney){
        String sql = "select ";
        if(NBT){
            sql += "NBT";
        }
        if(scores){
            if(!sql.equals("select ")) sql += ",";
            sql += "scores";
        }
        if(tags){
            if(!sql.equals("select ")) sql += ",";
            sql += "tags";
        }
        if(LLMoney){
            if(!sql.equals("select ")) sql += ",";
            sql += "LLMoney";
        }
        if(sql.equals("select ")) return null;

        sql += " from PlayerData where xuid ='" + xuid +"'";
        return jdbcTemplate.queryForList(sql);
    }
    @Override
    // 设置指定玩家的Data 没有记录则返回void
    public String setPlayerData(String xuid, String NBT,String scores,String tags,String LLMoney){
        String sql = "update PlayerData set ";

        if(!NBT.equals("false")){
            sql += "NBT='" + NBT + "'";
        }
        if(!scores.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "scores='" + scores + "'";
        }
        if(!tags.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "tags='" + tags + "'";
        }
        if(!LLMoney.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "LLMoney='" + LLMoney + "'";
        }
        // void set
        if(sql.equals("update PlayerData set ")) return "success";

        sql += "where xuid='" + xuid + "'";
        int result = jdbcTemplate.update(sql);
        if(result == 1){
            return "success";
        }
        else{
            return "failed";
        }
    };
    @Override
    // 新增一个玩家Data的记录
    public String insertPlayerData(String xuid, String NBT, String scores, String tags, String LLMoney){
        String sql1 = "insert into PlayerData(xuid";
        String sql2 = "values ('" + xuid + "'";

        if(!NBT.equals("false")){
            sql1 += ",NBT";
            sql2 += ",'" + NBT + "'";
        }
        if(!scores.equals("false")){
            sql1 += ",scores";
            sql2 += ",'" + scores + "'";
        }
        if(!tags.equals("false")){
            sql1 += ",tags";
            sql2 += ",'" + tags + "'";
        }
        if(!LLMoney.equals("false")){
            sql1 += ",LLMoney";
            sql2 += ",'" + LLMoney + "'";
        }
        sql1 += ") ";
        sql2 += ")";
        int result = jdbcTemplate.update(sql1 + sql2);
        if(result == 1){
            return "success";
        }
        else{
            return "failed";
        }
    }
}
