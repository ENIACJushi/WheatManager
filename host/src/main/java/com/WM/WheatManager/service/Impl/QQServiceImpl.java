package com.WM.WheatManager.service.Impl;

import com.WM.WheatManager.dao.QQDao;
import com.WM.WheatManager.service.QQService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class QQServiceImpl implements QQService {
    @Autowired
    QQDao qqDao;

    @Override
    // 创建记录
    public String insertData(String name, long number){
        if(qqDao.insertData(name, number)){
            return "success";
        }
        else{
            if(qqDao.getName(number) != null){
                return "failed(existed)";
            }
        }
        return "failed";
    }
    @Override
    // 删除记录
    public String removeData(long number){
        if(qqDao.removeData(number)){
            return "success";
        }
        return "failed";
    }
    @Override
    // 设置QQ号
    public String setNumber(String name, long number){
        if(qqDao.setNumber(name, number)){
            return "success";
        }
        return "failed";
    }
    @Override
    // 设置玩家名
    public String setName(long number, String name){
        if(qqDao.setName(number, name)){
            return "success";
        }
        return "failed";
    }
    @Override
    // 获取玩家名对应的qq号
    public long getNumber(String name){
        Integer result = qqDao.getNumber(name);
        if(result == null){
            return -1;
        }
        else{
            return result.longValue();
        }
    }
    @Override
    // 获取qq号对应的玩家名
    public String getName(long number){
        String result = qqDao.getName(number);
        if(result == null){
            return "null";
        }
        else{
            return result;
        }
    }


}
