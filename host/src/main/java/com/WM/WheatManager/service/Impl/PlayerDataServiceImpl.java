package com.WM.WheatManager.service.Impl;

import com.WM.WheatManager.dao.PlayerDataDao;
import com.WM.WheatManager.service.PlayerDataService;
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
    public List<Map<String, Object>> queryPlayerData(String xuid, boolean bag, boolean enderChest, boolean attributes, boolean level, boolean scores, boolean tags, boolean money){
        String sql = "select ";
        if(bag){
            sql += "bag";
        }
        if(enderChest){
            if(!sql.equals("select ")) sql += ",";
            sql += "ender_chest";
        }
        if(attributes){
            if(!sql.equals("select ")) sql += ",";
            sql += "attributes";
        }
        if(level){
            if(!sql.equals("select ")) sql += ",";
            sql += "level";
        }
        if(scores){
            if(!sql.equals("select ")) sql += ",";
            sql += "scores";
        }
        if(tags){
            if(!sql.equals("select ")) sql += ",";
            sql += "tags";
        }
        if(money){
            if(!sql.equals("select ")) sql += ",";
            sql += "money";
        }
        if(sql.equals("select ")) return null;

        sql += " from PlayerData where xuid ='" + xuid +"'";
        return jdbcTemplate.queryForList(sql);
    }
    @Override
    // 设置指定玩家的Data 没有记录则返回void
    public String setPlayerData(String xuid, String bag, String enderChest, String attributes, String level, String scores, String tags, Integer money){
        String sql = "update PlayerData set ";

        if(!bag.equals("false")){
            sql += "bag='" + bag + "'";
        }
        if(!enderChest.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "ender_chest='" + enderChest + "'";
        }
        if(!attributes.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "attributes='" + attributes + "'";
        }
        if(!level.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "level='" + level + "'";
        }
        if(!scores.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "scores='" + scores + "'";
        }
        if(!tags.equals("false")){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "tags='" + tags + "'";
        }
        if(money != -9961){
            if(!sql.equals("update PlayerData set ")) sql += ",";
            sql += "money=" + money;
        }
        // void set
        if(sql.equals("update PlayerData set ")) return "success";

        sql += "where xuid='" + xuid + "'";
        int result = jdbcTemplate.update(sql);
        if(result == 1){
            return "success";
        }
        else{
            return insertPlayerData(xuid, bag, enderChest, attributes, level, scores, tags, money);
        }
    };
    @Override
    // 新增一个玩家Data的记录
    public String insertPlayerData(String xuid, String bag, String enderChest, String attributes, String level, String scores, String tags, Integer money){
        String sql1 = "insert into PlayerData(xuid";
        String sql2 = "values ('" + xuid + "'";

        if(!bag.equals("false")){
            sql1 += ",bag";
            sql2 += ",'" + bag + "'";
        }
        if(!enderChest.equals("false")){
            sql1 += ",ender_chest";
            sql2 += ",'" + enderChest + "'";
        }
        if(!attributes.equals("false")){
            sql1 += ",attributes";
            sql2 += ",'" + attributes + "'";
        }
        if(!level.equals("false")){
            sql1 += ",level";
            sql2 += ",'" + level + "'";
        }
        if(!scores.equals("false")){
            sql1 += ",scores";
            sql2 += ",'" + scores + "'";
        }
        if(!tags.equals("false")){
            sql1 += ",tags";
            sql2 += ",'" + tags + "'";
        }
        if(money == -9961){
            sql1 += ",money";
            sql2 += "," + money;
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
