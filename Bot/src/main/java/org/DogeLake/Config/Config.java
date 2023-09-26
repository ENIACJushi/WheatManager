package org.DogeLake.Config;

public class Config {
    public static long botNumber = 1366856450;
    public static class groupId{
        public static long player = 688524595;
        public static long admin  = 221749849;
        public static long debug  = 830025085;
        public static boolean isGroup(long id){
            if(id == player || id == admin || id == debug){
                return true;
            }
            else{
                return false;
            }
        }
    }

}
