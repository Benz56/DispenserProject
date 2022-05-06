package com.beehive.labsafety.mqtt;

import com.fasterxml.jackson.databind.ObjectMapper;
import lombok.Getter;
import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MqttDefaultFilePersistence;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.stereotype.Service;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.UUID;

@Service
public class MqttClientService {

    private final Logger logger = LoggerFactory.getLogger(MqttClientService.class);
    private final String mqttServerUrl = "tcp://simonmdsn.com:1883";
    @Getter
    private MqttClient client;
    private final ObjectMapper mapper;

    public MqttClientService(ObjectMapper mapper) {
        this.mapper = mapper;        var clientUUID = UUID.randomUUID();
        if (!Files.exists(Path.of("./mqtt-client"))) {
            try {
                Files.createDirectory(Path.of("./mqtt-client"));
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        try {
            client = new MqttClient(mqttServerUrl, clientUUID.toString(), new MqttDefaultFilePersistence("./mqtt-client"));

            MqttConnectOptions options = new MqttConnectOptions();
            options.setAutomaticReconnect(true);
            options.setCleanSession(true);
            options.setConnectionTimeout(10);

            if (!client.isConnected()) {
                logger.info(" --- MQTT CLIENT --- Connecting to MQTT broker at " + mqttServerUrl);
                client.connect(options);
            }
            if (client.isConnected()) {
                logger.info(" --- MQTT CLIENT --- Established connection to MQTT broker at " + mqttServerUrl);
            }
        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

//    @Scheduled(fixedDelay = 10000)
//    public void sendMessage() {
//        try {
//            client.publish("/snus", new MqttMessage("1".getBytes(StandardCharsets.UTF_8)));
//        } catch (MqttException e) {
//            e.printStackTrace();
//        }
//    }

}
