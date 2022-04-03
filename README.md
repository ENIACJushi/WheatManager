# MSM


α测试中...... 变动实在太多了，仅供参考，若要使用则应有网络应用开发经验。



MSM，全称MinecraftServerManager，主要用于管理多个MC服务器间的数据交互，以及外部应用对他们的访问。

下面我将一一介绍这个项目的进展、安装以及声明：



一、当前进展

    1. 平台方面，构建了两个端，一个是JS写的LiteLoader插件端，另一个是用Java写的群组服管理端。未来也许会增加安卓端和网页端，毕竟群组服管理端使用的框架SpringBoot很适合做这些；

    2. 功能方面，实现了多服玩家数据同步。游戏数据的读取和设置参考背包同步插件“WSClient-Syn”（如果只是要同步功能，不考虑web应用的话建议用这个插件）。未来计划增加跨服指令、实名认证、防沉迷、自助白名单获取和通过外部程序调用MC操作等功能。





二、安装说明

这部分真的是可以写非常长......没耐心可以返回上一页了。

目前仅有windows下的安装教程。



    1. 下载并配置Java

    注意：只需要在管理端所在的服务器做这一步。

    ① 下载链接：https://www.oracle.com/java/technologies/javase/jdk14-archive-downloads.html

    ② 找到windows对应的exe文件下载；

    ③ 双击执行exe，记住安装路径；

    ④ 配置环境变量。依次选择我的电脑—右键属性—选择高级系统设置—环境变量

       a, 在系统变量处选择新建，变量名“JAVA_HOME”，变量值为上一步的安装路径；

       b. 在系统变量处选择新建，变量名“CLASSPATH”，变量值“%JAVA_HOME%\lib\dt.jar;%JAVA_HOME%\lib\tools.jar”；

       c. 在系统变量处双击Path，新增变量值“%JAVA_HOME%\bin”和“%JAVA_HOME%\jre\bin”。

    ⑤ 确认安装。打开cmd命令行，输入java -version，若出现java的版本信息则说明安装成功。



    2. 安装并配置SQLServer2019

   注意：只需要在一个服务器做这一步，可以不和管理端在同一服务器。

    【安装部分】

    ① 下载链接：https://www.microsoft.com/en-us/evalcenter/evaluate-sql-server-2019

    ② 选择SQLServer2019 - Get started for free - EXE - [continue]，填写资料再点击[Continue]，然后[download]；

    ③ 双击下载下来的文件，选择基本安装，接受协议，选择语言为中文，等待安装完成。

    ④ 安装完成，点击“安装SSMS”。



   【配置部分】



    ① 使用SSMS连接数据库；



    ② 在主界面左侧“对象资源管理器”，右键“数据库”，点击“新建数据库”，数据库名称填“MSM”，确定；



    ③ 在主界面顶栏点击“新建查询”，粘贴如下代码并点击“执行”；



[CODE=sql]use MSM

Create table Player(

xuid varchar(128) primary key,

uuid varchar(128),

real_name varchar(64),

nick_name varchar(64),

online varchar(64)

)

Create table PlayerData(

xuid varchar(128) primary key,

NBT text,

scores text,

tags text,

money Integer

)[/CODE]



    ④ 在主界面左侧“对象资源管理器”，展开“安全性”，右键“登录名”，单击“新建登录名”：

    a. 输入一个登录名，为方便之后的演示，我这里是“root”；

    b. 选择“SQL Server身份验证”，输入密码并确认密码，我这里是“database2333”；

    c. 取消勾选“强制执行密码策略”；

    d. 在靠左的选择页点击“服务器角色”，勾选“sysadmin”；

    e. 在靠左的选择页点击“状态”，勾选“授予”、“启用”；

    f. 点击右下角的“确定”创建登录名。



    ⑤ 在主界面左侧“对象资源管理器”，右键最上面的根节点，点击“属性”，在弹出的对话框的左侧选择页单击“安全性”，选择“SQL Server和Windows身份验证模式”，确定；



    ⑥ 退出SSMS，打开‘'SQL Server 2019 配置管理器“：

    a. 在左侧导航栏，展开”SQL Server网络配置“，点击”SQLEXPRESS 的协议“；

    b. 右键”TCP/IP“，启用协议，然后双击进入属性对话框；

    c. 选择”IP地址“，拉到最下面，把”IPALL“的”TCP“端口改为1433，确定。

    d. 在左侧导航栏，点击”SQL Server服务“，右键”SQL Server(SQLEXPRESS)“， 重启。



    ⑦ 测试：打开SSMS，选择SQL Server身份验证，用刚刚创建的登录名“root”-“database2333”登录，连接成功即为测试完成。



    3.配置管理端

    找到并打开”MinecraftServerManager\application.properties“，设置如下参数：

 

[CODE]spring.socket.serverKey=[要设置的web socket服务器密码]

spring.datasource.server=[数据库（SQL Server）所在服务器的外网IP]

spring.datasource.username=[SQL Server的登录名]

spring.datasource.password=[SQL Server的密码][/CODE]



    4.配置插件端

    ① 将MinecraftServerManager.js复制到”plugins“文件夹；

    ② 启动服务器，然后关闭；

    ③ 修改”plugins\MinecraftServerManager\config.json“：

[CODE=json]{

    "hostURL": "localhost:8080", // localhost为管理端所在的服务器的IP，若该MC服务器与管理端在同一主机，则为localhost

    "serverName": "ZY",          // 服务器名称

    "key": "S@H12=S-NA",         // web socket密码

    "timeOut": 15,               // 自动重连时间间隔

    "autoSaveTime": 5,           // 自动保存间隔

    "synNBT": true,              // 同步NBT

    "synScores": true,           // 同步计分板

    "synTags": true,             // 同步标签

    "synLLMoney": true           // 同步LLMoney

}[/CODE]



    5.测试

    ① 将”start server.bat“和”MinecraftServerManager.jar“放在同一目录，双击”start server.bat“

    ② 启动MC服务器，看到如下信息说明连接成功：

[CODE]INFO [MSM] Connection established.

INFO [MSM] Authentication passed.[/CODE]

    ③ 启动Minecraft，进行实地测试。



三、声明

    1. 萌新，第一个插件。起初目的是构建一个稳定的背包同步顺便练练springboot和js，所以部分设计并不完美；

    2. 我的专业方向是人工智能，而这个项目嘛偏向物联网，所以只是出于兴趣写着玩玩，出现一些超菜超怪的操作也在情理之中；

    3. 安装遇到问题不要找作者，应该找百度，上CSDN，关于如何安装的问题一律不答；

    4. 很久没练英语了，提示信息和注释可能会有一些语法错误；

    5. 反馈渠道：评论区、QQ 368364534。加QQ先说明来意，非必要不Q；

    6. 参考支持方式：贡献代码、来服玩。内测在该服务器进行，小版本也会优先发到服群，群号：688524595。
