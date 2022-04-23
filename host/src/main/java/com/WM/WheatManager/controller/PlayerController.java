package com.WM.WheatManager.controller;

import com.WM.WheatManager.service.PlayerDataService;
import com.alibaba.fastjson.JSONObject;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;
import java.util.Map;

@RestController
public class PlayerController {
    @Autowired
    PlayerDataService playerDataService;
    // 获取玩家数据
    @RequestMapping("/getPlayerData")
    public String hello(@RequestParam("xuid")String xuid, @RequestParam("bag")String bag, @RequestParam("enderChest")String enderChest,
                        @RequestParam("attributes")String attributes, @RequestParam("level")String level,
                        @RequestParam("scores")String scores, @RequestParam("tags")String tags,
                        @RequestParam("money")String money) {
        JSONObject reply = new JSONObject();
        reply.put("xuid", xuid);
        List<Map<String, Object>> data = playerDataService.queryPlayerData(xuid, bag.equals("true"),enderChest.equals("true"),attributes.equals("true"),
                level.equals("true"), scores.equals("true"), tags.equals("true"), money.equals("true"));
        if (data == null || data.isEmpty()) {
            reply.put("result", "void");
        }
        else {
            reply.put("result", "success");
            reply.put("bag",        data.get(0).get("bag"));
            reply.put("enderChest", data.get(0).get("ender_chest"));
            reply.put("attributes", data.get(0).get("attributes"));
            reply.put("level",      data.get(0).get("level"));
            reply.put("scores",     data.get(0).get("scores"));
            reply.put("tags",       data.get(0).get("tags"));
            reply.put("money",      data.get(0).get("money"));
        }
        return reply.toJSONString();
    }
}