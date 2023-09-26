package org.DogeLake;

import net.mamoe.mirai.Bot;
import net.mamoe.mirai.BotFactory;
import net.mamoe.mirai.console.plugin.jvm.JavaPlugin;
import net.mamoe.mirai.console.plugin.jvm.JvmPluginDescriptionBuilder;
import net.mamoe.mirai.contact.Member;
import net.mamoe.mirai.contact.MemberPermission;
import net.mamoe.mirai.event.events.FriendMessageEvent;
import net.mamoe.mirai.event.events.GroupMessageEvent;
import net.mamoe.mirai.event.events.MemberJoinEvent;
import net.mamoe.mirai.message.data.MessageChainBuilder;
import net.mamoe.mirai.message.data.QuoteReply;
import net.mamoe.mirai.utils.BotConfiguration;
import net.mamoe.mirai.utils.MiraiLogger;
import org.DogeLake.Config.Config;
import org.DogeLake.webSocket.MyWebSocketClient;
import org.java_websocket.enums.ReadyState;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.*;

public final class Plugin extends JavaPlugin {
    public static String channel = "sg";
    public static final Plugin INSTANCE = new Plugin();
    public static final String botName = "扶栏躲鹿";
    public static final String commandStr ="doge";

    public static String repeater = "";
    public static int  repeater_counter = 0;

    public static MyWebSocketClient myClient;
    public static MiraiLogger logger;
    static long yourQQNumber = 1366856450;

    static long[] groupNumber = {830025085, 688524595};
    static boolean isGroup(long number){
        for(long i : groupNumber){
            if(number == i) return true;
        }
        return false;
    }

    private Plugin() {
        super(new JvmPluginDescriptionBuilder("org.DogeLake.plugin", "1.0-SNAPSHOT")
                .name("DogeLake")
                .author("EJ")
                .build());
        Bot bot = BotFactory.INSTANCE.newBot(1366856450, "2019-pef",new BotConfiguration() {{
            // ANDROID_PHONE，ANDROID_PAD，ANDROID_WATCH，IPAD，MACOS
            setProtocol(MiraiProtocol.ANDROID_PAD);

        }});
        bot.login();
        logger = getLogger();
        // 连接服务器后端
        try{

            myClient = new MyWebSocketClient(new URI("ws://localhost:20189/ws/asset"));
            myClient.connect();
            while (!myClient.getReadyState().equals(ReadyState.OPEN)) {
                System.out.println("连接中。。。");
                Thread.sleep(1000);
            }
            //如果断线，则重连并重新发送验证信息
            Timer t = new Timer();
            t.scheduleAtFixedRate(new TimerTask() {
                @Override
                public void run() {
                    if (!myClient.getReadyState().equals(ReadyState.OPEN)) {
                        myClient.reconnect();
                    }
                }
            },2000,5000);//5秒执行一次 然后休息2秒

        } catch (Exception e) {
            e.printStackTrace();
        }
        Plugin.afterLogin(bot);
    }

    @Override
    public void onEnable() {
        getLogger().info("Plugin loaded!");
    }
    // 机器人登录成功
    public static void afterLogin(Bot bot) {

        // 欢迎信息
        bot.getEventChannel().subscribeAlways(MemberJoinEvent.class, (event) -> {
            event.getGroup().sendMessage(new MessageChainBuilder().append("欢迎新人！ 请先阅读群公告 ~\n 发送\"加白名单 游戏名\"绑定白名单；  \n搜索\"o80w0e7983\"加入服频道，获取更新日志、阅读与书写服务器历史！").build());
        });
        // 私聊信息
        bot.getEventChannel().subscribeAlways(FriendMessageEvent.class, (event) -> {
            if (event.getSender().getId() == yourQQNumber) {
                event.getSubject().sendMessage(new MessageChainBuilder()
                        .append(new QuoteReply(event.getMessage()))
                        .append("Hi, you just said: '")
                        .append(event.getMessage())
                        .append("'")
                        .build()
                );
            }
        });
        // 群聊信息(doge lake)
        bot.getEventChannel().subscribeAlways(GroupMessageEvent.class, (event) -> {
            if (Config.groupId.isGroup(event.getGroup().getId())) {
                logger.info(event.getMessage().contentToString());
                String msg = event.getMessage().contentToString();
                Member sender = event.getSender();
                while(true){
                    // 复读机
                    if(repeater.equals(msg)) {
                        repeater_counter ++;
                        if(repeater_counter == 2) {
                            event.getSubject().sendMessage(new MessageChainBuilder().append(msg).build());
                        }
                    }
                    else{
                        repeater = msg;
                        repeater_counter = 0;
                    }
                    // 禁止小冰
                    if(event.getSender().getId() == 2854196306L){
                        event.getSender().mute(3600);
                    }
                    if (msg.contains("@2854196306")) {
                        event.getGroup().getMembers();
                        for (Member member : event.getGroup().getMembers()) {
                            if (member.getId() == 2854196306L) {
                                member.mute(3600);
                                break;
                            }
                        }
                        event.getSubject().sendMessage(new MessageChainBuilder().append("禁 止 小 冰").build());
                    }
                    // 问候
                    if(good(msg, event, sender))               break;
                    // 必触发动态指令
                    if(containMsgMust(msg, event, sender))     break;
                    // 1/5概率触发
                    if (rollExecute(1,5)) {
                        if(dynamicCommand(msg, event, sender)) break; // 动态指令
                        if(staticCommand(msg, event, sender))  break; // 静态指令
                        if(containMsg(msg, event, sender))     break; // 关键词识别
                    }
                    break;
                }
                // 服务器信息同步
                if(Config.groupId.player == event.getGroup().getId()){
                    myClient.broadcastMessage("chat", sender.getNick(), msg, channel);
                }
            }
        });
    }

    ///// 外部接口 /////
    // 向某群发送信息
    public static void sendMessage(long groupID, String message){
        Bot.getInstance(Config.botNumber).getGroup(groupID).sendMessage(new MessageChainBuilder().append(message).build());
    }

    ///// 作为聊天机器人 /////
    // 关键词识别 没有doge时1/5概率触发
    public static boolean containMsg(String msg, GroupMessageEvent event, Member sender){
        if(msg.contains("服") && (msg.contains("炸了") || msg.contains("崩了"))) {
            event.getSubject().sendMessage(new MessageChainBuilder().append("gg").build()); return true;}
        if(msg.contains("红魔馆") && (msg.contains("爆") || msg.contains("炸"))) {
            event.getSubject().sendMessage(new MessageChainBuilder().append("干得好！").build()); return true;}
        return false;
    }
    public static boolean containMsgMust(String msg, GroupMessageEvent event, Member sender) {
        // 管理群
        if(event.getGroup().getId() == Config.groupId.admin
                || event.getSender().getPermission() == MemberPermission.ADMINISTRATOR
                || event.getSender().getPermission() == MemberPermission.OWNER ){
            // 命令 目标 命令字符串
            if(msg.length() > 4 && msg.substring(0,3).equals("命令 ")){
                String info = msg.substring(msg.indexOf(" ")+1);
                myClient.pushCommand(event.getGroup().getId(), info.substring(0, info.indexOf(" ")),
                        info.substring(info.indexOf(" ")+1), "direct");
                return true;
            }
            if(msg.equals("打晕 server")){
                channel = "group";
                event.getSubject().sendMessage(new MessageChainBuilder().append("[server] 我晕了").build());
            }
            if(msg.equals("救醒 server")){
                channel = "sg";
                event.getSubject().sendMessage(new MessageChainBuilder().append("[server] 我醒了").build());
            }
        }
        if (msg.contains("[测试]信息内容")) {
            event.getSubject().sendMessage(new MessageChainBuilder().append(msg).build());
            return true;
        }
        else if(msg.length() > 4 && msg.substring(0,5).equals("改白名单 ")){
            myClient.whiteList(event.getGroup().getId(), msg.substring(10), event.getSender().getId(), "change");
            return true;
        }
        else if(msg.length() > 4 && msg.substring(0,5).equals("加白名单 ")){
            myClient.whiteList(event.getGroup().getId(), msg.substring(10), event.getSender().getId(), "insert");
            return true;
        }
        else if(msg.length() > 3 && msg.substring(0,4).equals("删白名单")){
            myClient.whiteList(event.getGroup().getId(), "null", event.getSender().getId(), "delete");
            return true;
        }
        return false;
    }
    // 静态指令 1/5概率触发
    public static boolean staticCommand(String msg, GroupMessageEvent event, Member sender){
        switch (msg){
            case "hello doge":  event.getSubject().sendMessage(new MessageChainBuilder().append("Hello Doge!").build()); return true;
            case "大家好":       event.getSubject().sendMessage(new MessageChainBuilder().append("0 0").build());         return true;
            case "来点色图":      event.getSubject().sendMessage(new MessageChainBuilder().append("你在想peach").build());  return true;
            case "？": case "?": event.getSubject().sendMessage(new MessageChainBuilder().append(new QuoteReply(event.getMessage())).append("？").build());return true;
            case "草":           event.getSubject().sendMessage(new MessageChainBuilder().append("草").build());  return true;
            default: return false;
        }
    }
    // 动态指令 没有doge时1/5概率触发
    public static boolean dynamicCommand(String msg, GroupMessageEvent event, Member sender){
        // 随机数
        if(msg.length()>=4 && msg.substring(0,4).equals("roll")){
            event.getSubject().sendMessage(new MessageChainBuilder().append(Integer.toString((int)(Math.random() * 5))).build());
            return true;
        }
        return false;
    }
    // 早中晚问候 没有doge时1/5概率触发
    static HashSet<Member> goodMorningMember = new HashSet<Member>();
    static HashSet<Member> goodMorningMember_2 = new HashSet<Member>();
    static HashSet<Member> goodMorningMember_3 = new HashSet<Member>();
    static HashSet<Member> goodMorningMember_4 = new HashSet<Member>();
    public static boolean good(String msg, GroupMessageEvent event, Member sender){
        int morning = 60;
        int noon    = 1800;
        int night   = 32400;
        // 管理员/群主
        if(event.getSender().getPermission() == MemberPermission.ADMINISTRATOR || event.getSender().getPermission() == MemberPermission.OWNER ){
            if(msg.contains(" ")){
                if(msg.length() > 3){
                    if(msg.substring(0,3).equals("晚安@")){
                        int memberID = Integer.parseInt(msg.substring(3, msg.indexOf(" ")));
                        for(Member member :event.getGroup().getMembers()){
                            if(member.getId() == memberID){
                                member.mute(night);
                                event.getSubject().sendMessage(new MessageChainBuilder().append("晚安! 你获得的睡眠时间：9h").build());
                                return true;
                            }
                        }
                        return true;
                    }
                    if(msg.substring(0,3).equals("午安@")){
                        int memberID = Integer.parseInt(msg.substring(3, msg.indexOf(" ")));
                        for(Member member :event.getGroup().getMembers()){
                            if(member.getId() == memberID){
                                member.mute(noon);
                                event.getSubject().sendMessage(new MessageChainBuilder().append("午安! 你获得的睡眠时间：1min").build());
                                return true;
                            }
                        }
                        return true;
                    }
                    if(msg.substring(0,3).equals("刷牙@")){
                        int memberID = Integer.parseInt(msg.substring(3, msg.indexOf(" ")));
                        for(Member member :event.getGroup().getMembers()){
                            if(member.getId() == memberID){
                                member.mute(morning);
                                event.getSubject().sendMessage(new MessageChainBuilder().append("刷吧!").build());
                                return true;
                            }
                        }
                        return true;
                    }
                    if(msg.substring(0,3).equals("打晕@")){
                        int memberID = Integer.parseInt(msg.substring(3, msg.indexOf(" ")));
                        for(Member member :event.getGroup().getMembers()){
                            if(member.getId() == memberID){
                                int time = 1;
                                String timeString = msg.substring(msg.indexOf(" ") + 1);
                                if(!timeString.equals("")){
                                    time = Integer.parseInt(timeString);
                                }
                                member.mute(time * 60);
                                event.getSubject().sendMessage(new MessageChainBuilder().append(member.getNick()).append("被打晕了！TA将在")
                                        .append(String.valueOf(time)).append("分钟后苏醒").build());
                                return true;
                            }
                        }
                        return true;
                    }
                }
            }
        }
        if(msg.length()>3){
            if(msg.substring(0,3).equals("打晕@")){
                if(msg.contains(" ")){
                    int memberID = Integer.parseInt(msg.substring(3, msg.indexOf(" ")));
                    for(Member member :event.getGroup().getMembers()){
                        if(member.getId() == memberID){
                            int time = 1;
                            member.mute(time * 60);
                            event.getSubject().sendMessage(new MessageChainBuilder().append(member.getNick()).append("被打晕了！TA将在")
                                    .append(String.valueOf(time)).append("分钟后苏醒").build());
                            return true;
                        }
                    }
                }
                return true;
            }
        }
        // 早安 - 吃早餐
        if(goodMorningMember_4.contains(sender)){
            goodMorningMember_4.remove(sender);
            if(msg.equals("吃过了")){
                event.getSender().mute(1200);
                event.getSubject().sendMessage(new MessageChainBuilder().append("再吃一顿！").build());                    // bad ending 7
            }
            else if(msg.contains("不") || msg.contains("no")){
                event.getSender().mute(1);
                event.getSubject().sendMessage(new MessageChainBuilder().append("怎么能不吃饭呢？ （塞）").build());
                event.getSubject().sendMessage(new MessageChainBuilder().append("*在"+botName+"的大力塞包下，进食速度大大提升了！*").build());  // bad ending 9
            }
            else if(msg.contains("要") || msg.contains("吃")){
                event.getSender().mute(600);
                event.getSubject().sendMessage(new MessageChainBuilder().append("吃吧！ 这是从东土大唐而来的人肉包子 ~ ").build()); // bad ending 8
            }
            else{
                event.getSender().mute(1800);
                event.getSubject().sendMessage(new MessageChainBuilder().append("居然不理我？ 回去睡觉吧！").build());        // bad ending 3
            }
            return true;
        }
        // 早安 - 上撤硕
        if(goodMorningMember_3.contains(sender)){
            goodMorningMember_3.remove(sender);
            if(msg.equals("上过了")){
                event.getSubject().sendMessage(new MessageChainBuilder().append("需要吃早餐吗？").build());                 // next step
                goodMorningMember_4.add(sender);
            }
            else if(msg.contains("不") || msg.contains("no")){
                event.getSubject().sendMessage(new MessageChainBuilder().append("以后再上也不迟 ~").build());               // good ending
                event.getSubject().sendMessage(new MessageChainBuilder().append("*获得称号「便秘者」*").build());
            }
            else if(msg.contains("要") || msg.contains("上")){
                event.getSender().mute(300);
                event.getSubject().sendMessage(new MessageChainBuilder().append("祝您大便通畅").build());                   // bad ending 6
            }
            else{
                event.getSender().mute(1800);
                event.getSubject().sendMessage(new MessageChainBuilder().append("居然不理我？ 回去睡觉吧！").build());        // bad ending 3
            }
            return true;
        }
        // 早安 - 洗脸
        if(goodMorningMember_2.contains(sender)){
            goodMorningMember_2.remove(sender);
            if(msg.equals("洗过了")){
                event.getSubject().sendMessage(new MessageChainBuilder().append("需要上厕所吗？").build());                 // next step
                goodMorningMember_3.add(sender);
            }
            else if(msg.contains("不") || msg.contains("no")){
                event.getSender().mute(60);
                event.getSubject().sendMessage(new MessageChainBuilder().append("怎么能不洗脸呢！").build());               // bad ending 5
            }
            else if(msg.contains("要") || msg.contains("洗")){
                event.getSender().mute(60);
                event.getSubject().sendMessage(new MessageChainBuilder().append("洗吧！ 这是从血池地狱捞来的石油！").build()); // bad ending 4
            }
            else{
                event.getSender().mute(1800);
                event.getSubject().sendMessage(new MessageChainBuilder().append("居然不理我？ 回去睡觉吧！").build());        // bad ending 3
            }
            return true;
        }
        // 早安 - 刷牙
        if(goodMorningMember.contains(sender)){
            goodMorningMember.remove(sender);
            if(msg.equals("刷过了")){
                event.getSubject().sendMessage(new MessageChainBuilder().append("需要洗脸吗？").build());           // next step
                goodMorningMember_2.add(sender);
            }
            else if(msg.contains("不") || msg.contains("no")){
                event.getSender().mute(morning);
                event.getSubject().sendMessage(new MessageChainBuilder().append("怎么能不刷牙呢？").build());        // bad ending 2
            }
            else if(msg.contains("要") || msg.contains("刷")){
                event.getSender().mute(morning);
                event.getSubject().sendMessage(new MessageChainBuilder().append("刷吧！").build());                // bad ending 1
            }
            else{
                event.getSender().mute(1800);
                event.getSubject().sendMessage(new MessageChainBuilder().append("居然不理我？ 回去睡觉吧！").build()); // bad ending 3
            }
            return true;
        }
        // 早安 - 0 午安 晚安
        boolean runCmd = false;
        if(msg.contains(commandStr)){
            msg = msg.replace(commandStr, "");
            msg = msg.replace(" ", "");
            runCmd = true;
        }
        else if(rollExecute(1, 5)){
            runCmd = true;
        }
        if(runCmd){
            if(msg.equals("早") || msg.contains("早安") || msg.contains("早上好") || msg.contains("早上晚")) {
                goodMorningMember.add(sender);
                event.getSubject().sendMessage(new MessageChainBuilder().append("早安! 需要刷牙吗？").build());
                return true;
            }
            // 午安
            if(msg.contains("午安")){
                if (msg.contains(commandStr) || rollExecute(1,5)) {
                    event.getSender().mute(noon);
                    event.getSubject().sendMessage(new MessageChainBuilder().append("午安! 你获得的睡眠时间：30min").build());
                }
                return true;
            }
            // 晚安
            if(msg.contains("晚安")){
                if (msg.contains(commandStr) || rollExecute(1,5)) {
                    event.getSender().mute(night);
                    event.getSubject().sendMessage(new MessageChainBuilder().append("晚安! 你获得的睡眠时间：9h").build());
                }
                return true;
            }
        }
        return false;
    }

    public static boolean rollExecute(int a, int b){
        return (int)(Math.random() * b) < a;
    }
}