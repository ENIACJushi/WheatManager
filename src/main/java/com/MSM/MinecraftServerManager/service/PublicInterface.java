// 公用接口
// 可以被多个用例使用的接口放在这里
package com.MSM.MinecraftServerManager.service;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class PublicInterface {
    // 验证管理员身份
    // 在进行管理员操作前使用
    @RequestMapping("/interfaceName")
    public String interfaceName (){
        return "returnMsg";
    }
}
