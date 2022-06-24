// header required for bluetooth serial connection with ESP32
#include "BluetoothSerial.h"
#include <ESP32Servo.h>

// initialise the class
BluetoothSerial ESP_blueTooth;
Servo servo_motor;  // create servo object to control a servo

// initialise PINs on ESP32 to servoMotor
#define servo_Pin 14

// this is the variable to store int coming from bluetooth,
// tells whether to move servo clockwise/anticlockwise
int incoming;
int pos = 90;    // initial servo position

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Bluetooth Client Connected");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  ESP_blueTooth.register_callback(callback);
  
  if(!ESP_blueTooth.begin("ESP32")){; // name of the bluetooth interface, visible when scanning for bluetooth devices
    Serial.println("An error has occurred while initialising Bluetooth");
  }else{
    Serial.println("Bluetooth initialised");
    }
  servo_motor.setPeriodHertz(50);    // standard 50 hz servo motor
  servo_motor.attach(servo_Pin, 500, 2400); // attaches the servo on pin 14 of the ESP to the servo object
  // the default min/max of 1000us and 2000us
  // different servos may require different min/max settings
  // for an accurate 0 to 180 sweep
  servo_motor.write(pos);
}

void loop() {
  // put your main code here, to run repeatedly:

    if (ESP_blueTooth.available()){
      incoming = ESP_blueTooth.read(); // read the received data and store it into the variable incoming
      Serial.print("Data received: ");
      Serial.println(incoming);       // prints value/data received to Serial
      
      if ((incoming>=0) && (incoming<=180)){
        servo_motor.write(incoming);
      }
    }
}
