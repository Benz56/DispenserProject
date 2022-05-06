
import serial
import paho.mqtt.client as mqtt
from enum import Enum
import camera
import json

class Topic(Enum):
    DISPENSE_SNUS = "/dispense"
    PAYMENT_REQUEST = "/requestpayment"
    PAYMENT_RECIEVED = "/recievedpayment"

def dispenseSnus(type):
  ser = serial.Serial('/dev/ttyUSB0', 115200, timeout=1) # open serial port
  print(ser.name)
  ser.write(f'{type}\n'.encode())
  ser.close() 


def on_connect(client, userdata, flags, rc):
    print("Connected to mqtt server")
    client.subscribe(Topic.PAYMENT_REQUEST.value)
    client.subscribe(Topic.DISPENSE_SNUS.value)


def on_message(client, userdata, msg):
    topic = msg.topic
    if topic == Topic.DISPENSE_SNUS.value:
        print("Dispensing snus now")
        dispenseSnus(msg.payload.decode("utf-8"))
    if topic == Topic.PAYMENT_REQUEST.value:
        print("Awaiting your payment...")
        snus_type = msg.payload.decode("utf-8")
        hasPayed = camera.motion_detect()
        payload = json.dumps({"paid": hasPayed,"type": snus_type})
        client.publish(Topic.PAYMENT_RECIEVED.value, payload=payload)


if __name__ == "__main__":
  print("Initializing snus dispenser...")
  client = mqtt.Client()
  client.on_connect = on_connect
  client.on_message = on_message
  client.connect("REDACTED", 1883, 0)
  client.loop_forever()
  
