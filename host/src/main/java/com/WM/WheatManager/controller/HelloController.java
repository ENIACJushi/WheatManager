package com.WM.WheatManager.controller;

import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class HelloController {
    // 测试用接口
    @RequestMapping("/hello")
    public String hello(){
        return "哈喽！";
    }
}