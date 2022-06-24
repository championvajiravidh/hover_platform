// header required for bluetooth serial connection with ESP32
#include "BluetoothSerial.h"
#include <ESP32Servo.h>
#include <string>

// initialise the class
BluetoothSerial ESP_blueTooth;
Servo servo_motor_A;  // create servo object to control a servo


// initialise PINs on ESP32 to servoMotor
#define SERVO_PIN_A 14

// this is the variable to store int coming from bluetooth,
// tells whether to move servo clockwise/anticlockwise
int incoming;
int pos = 0;
String sub_string;
char s;
int sub_int;
int end_index = 1;

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Bluetooth Client Connected");
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
  Serial.println("");
  ESP_blueTooth.register_callback(callback);

  if (!ESP_blueTooth.begin("ESP32")) {
    ; // name of the bluetooth interface, visible when scanning for bluetooth devices
    Serial.println("An error has occurred while initialising Bluetooth");
  } else {
    Serial.println("Bluetooth initialised");
  }
  servo_motor_A.setPeriodHertz(50);    // standard 50 hz servo motor
  servo_motor_A.attach(SERVO_PIN_A, 500, 2400); // attaches the servo on pin 14 of the ESP to the servo object
  servo_motor_A.write(pos);
}

void loop() {
  // put your main code here, to run repeatedly:

  if (ESP_blueTooth.available()) {
    incoming = ESP_blueTooth.read(); // read the received data and store it into the variable incoming

    if ((incoming>=0) && (incoming<=20)){
      sub_int = (incoming* 9);
      servo_motor_A.write(sub_int);
      Serial.print("incoming: ");
      Serial.println(incoming);       // prints value/data received to Serial
      Serial.print("angle: ");
      Serial.println(sub_int);       // prints value/data received to Serial
    }
  }
}
