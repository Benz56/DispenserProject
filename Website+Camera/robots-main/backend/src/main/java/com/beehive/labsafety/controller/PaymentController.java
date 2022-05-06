package com.beehive.labsafety.controller;

import com.beehive.labsafety.mqtt.MqttClientService;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import lombok.RequiredArgsConstructor;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.CrossOrigin;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;

import java.nio.charset.StandardCharsets;
import java.util.List;

@Controller
@RequiredArgsConstructor
public class PaymentController {

    private final MqttClientService mqttClientService;

    private final Logger logger = LoggerFactory.getLogger(PaymentController.class);


    @PostMapping(value = "/order")
    public ResponseEntity<String> payment(@RequestParam String type) {
        logger.info("Received order request with type: " + type);
        if (!List.of("1", "2", "3").contains(type)) {
            return ResponseEntity.badRequest().body("Could not publish type parameter: " + type);
        }
        try {
            mqttClientService.getClient().publish("/snus", new MqttMessage(type.getBytes(StandardCharsets.UTF_8)));
        } catch (MqttException e) {
            e.printStackTrace();
            return ResponseEntity.badRequest().body("Could not publish type parameter: " + type);
        }
        return ResponseEntity.ok().build();
    }

    @CrossOrigin
    @PostMapping(value = "/dispense")
    public ResponseEntity<String> dispense(@RequestParam String type) {
        logger.info("Received dispense request with type: " + type);
        if (!List.of("1", "2", "3").contains(type)) {
            return ResponseEntity.badRequest().body("Could not publish type parameter: " + type);
        }
        try {
            mqttClientService.getClient().publish("/dispense", new MqttMessage(type.getBytes(StandardCharsets.UTF_8)));
        } catch (MqttException e) {
            e.printStackTrace();
            return ResponseEntity.badRequest().body("Could not publish type parameter: " + type);
        }
        return ResponseEntity.ok().build();
    }

    @CrossOrigin
    @PostMapping(value = "/requestpayment")
    public ResponseEntity<String> requestPayment(@RequestParam String type) {
        logger.info("Received requestpayment request with type: " + type);
        if (!List.of("1", "2", "3").contains(type)) {
            return ResponseEntity.badRequest().body("Could not publish type parameter: " + type);
        }
        try {
            mqttClientService.getClient()
                .publish("/requestpayment", new MqttMessage(type.getBytes(StandardCharsets.UTF_8)));
            //subscribeToReceivedPayment();
        } catch (MqttException e) {
            e.printStackTrace();
            return ResponseEntity.badRequest().body("Could not publish type parameter: " + type);
        }
        return ResponseEntity.ok().build();
    }

    private void subscribeToReceivedPayment() {
        logger.info("Subscribing to /receivedpayment topic.");
        try {
            CountDownLatch receivedSignal = new CountDownLatch(10);
            mqttClientService.getClient().subscribe("/receivedpayment", (topic, msg) -> {
                logger.info("Message received for topic '/receivedpayment'");
                String payload = new String(msg.getPayload(), StandardCharsets.UTF_8);
                logger.info("Message received for topic '/receivedpayment' with payload: " + payload);
                this.dispense(payload);
                receivedSignal.countDown();
            });

            receivedSignal.await(1, TimeUnit.MINUTES);
        } catch (InterruptedException | MqttException e) {
            e.printStackTrace();
        }
    }
}
