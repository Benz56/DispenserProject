package com.beehive.labsafety;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.scheduling.annotation.EnableScheduling;

@EnableScheduling
@SpringBootApplication
public class RobotDispenserApplication {

    public static void main(String[] args) {
        SpringApplication.run(RobotDispenserApplication.class, args);
    }

}
