package com.MSM.MinecraftServerManager.config;

import com.mchange.v2.c3p0.ComboPooledDataSource;
import com.microsoft.sqlserver.jdbc.SQLServerDataSource;
import org.mybatis.spring.annotation.MapperScan;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;

import java.beans.PropertyVetoException;
import java.sql.*;

/**
 * <p>
 * Description : DataSource 配置类
 * </p>
 *
 * @author : ENIAC_Jushi
 * @version : 1.0
 * @date : 2022.03
 */

@Configuration
@MapperScan("com.MSM.MinecraftServerManager.dao")
public class DataSourceConfiguration {

    @Value("${spring.datasource.driverClassName}")
    private String jdbcDriver;
    @Value("${spring.datasource.server}")
    private String server;
    @Value("${spring.datasource.port}")
    private int port;
    @Value("${spring.datasource.username}")
    private String jdbcUserName;
    @Value("${spring.datasource.password}")
    private String jdbcPassword;


    @Bean(name = "dataSource")
    public SQLServerDataSource createDataSource() throws PropertyVetoException
    {
        // Create datasource.
        SQLServerDataSource ds = new SQLServerDataSource();
        ds.setURL("jdbc:sqlserver://" + server +":"+ port +";DatabaseName=MSM;autoReconnectForPools=true;trustServerCertificate=true");
        ds.setUser(jdbcUserName);
        ds.setPassword(jdbcPassword);

//        try (Connection con = ds.getConnection();
//             CallableStatement cstmt = con.prepareCall("{call dbo.uspGetEmployeeManagers(?)}");) {
//            // Execute a stored procedure that returns some data.
//            cstmt.setInt(1, 50);
//            ResultSet rs = cstmt.executeQuery();
//
//            // Iterate through the data in the result set and display it.
//            while (rs.next()) {
//                System.out.println("EMPLOYEE: " + rs.getString("LastName") + ", " + rs.getString("FirstName"));
//                System.out.println("MANAGER: " + rs.getString("ManagerLastName") + ", " + rs.getString("ManagerFirstName"));
//                System.out.println();
//            }
//        }
//        // Handle any errors that may have occurred.
//        catch (SQLException e) {
//            e.printStackTrace();
//        }
        return ds;
    }
}
