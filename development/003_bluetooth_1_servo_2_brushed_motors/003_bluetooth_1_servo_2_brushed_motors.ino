// header required for bluetooth serial connection with ESP32
#include "BluetoothSerial.h"
#include <ESP32Servo.h>
#include <string>
#include <Wire.h>

// initialise the class
BluetoothSerial ESP_blueTooth;
Servo motor_H;  // create servo object to control a servo

// initialise PINs on ESP32 to servoMotor
#define SERVO_PIN_H 14

// define constants for H Bridge Motor Driver
// left motor
#define LIN1 19
#define LIN2 18
#define PWML 5

// right motor
#define RIN1 16
#define RIN2 17
#define PWMR 15

// this is the variable to store int coming from bluetooth,
int pos = 0;
int incoming;
int sub_int;

// constants for setting the max/min/neutral thresholds for XC-10A
const int max_sig = 162;
const int min_sig = 54;
const int neutral_sig = 90;

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
  motor_H.setPeriodHertz(50);    // standard 50 hz servo motor
  motor_H.attach(SERVO_PIN_H, 500, 2400); // attaches the servo on pin 14 of the ESP to the servo object
  //  Serial.println("Calibrating XC-10A Speed Controller Settings");
  //  esc_setup(motor_H);
  //  Serial.println("Calibration Complete");

  //set GPIO pins as OUTPUTS for LEFT MOTOR
  pinMode(LIN1, OUTPUT);
  pinMode(LIN2, OUTPUT);
  pinMode(PWML, OUTPUT);
  //set GPIO pins as OUTPUTS for RIGHT MOTOR
  pinMode(RIN1, OUTPUT);
  pinMode(RIN2, OUTPUT);
  pinMode(PWMR, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:

  if (ESP_blueTooth.available()) {
    incoming = ESP_blueTooth.read(); // read the received data and store it into the variable incoming

    // value pairs written as (incoming, sub_int)
    // min = (6, 54), max = (18, 162), neutral = (10, 90)
    // desired outcome of brushless motor is to hover up, not to stick down on surface,
    // hence eliminate [min,neutral) region, only allow controls in the [neutral, max] region
    if ((incoming >= 0) && (incoming <= 10)) {
      sub_int = (incoming + 10) * 8;
      Serial.println("HOVER");
      motor_H.write(sub_int);
    } else if ((incoming >= 11) && (incoming <= 36)) {
      sub_int = (incoming - 11) * 10;
      Serial.println("LEFT");
      runMotor_L(sub_int);

    } else if ((incoming >= 37) && (incoming <= 62)) {
      sub_int = (incoming - 37) * 10;
      Serial.println("RIGHT");
      runMotor_R(sub_int);
    }

    Serial.print("incoming: ");
    Serial.println(incoming);       // prints value/data received to Serial
    Serial.print("angle: ");
    Serial.println(sub_int);       // prints value/data received to Serial
  }
}

/*  void esc_setup(Servo motor) is a function to set up the range for the XC-10A speed controller.
    This will send the MAXIMUM throttle value, wait,
    send the MINIMUM throttle value, wait, and then the NEUTRAL value.

  void esc_setup(Servo motor) {
  int  i = 180;
  motor.write(i);
  delay(2500);
  Serial.println("Setting maximum throttle");
  for (i; i > max_sig; i){
    motor.write(i);
    delay(200);
  }
  delay(5000);
  Serial.println("Setting minimum throttle");
  motor.write(min_sig);
  delay(5000);
  Serial.println("Setting neutral throttle");
  motor.write(neutral_sig);
  delay(5000);
  }

  void esc_setup(Servo motor) {
  motor.write(max_sig);
  delay(5000);
  motor.write(min_sig);
  delay(5000);
  motor.write(neutral_sig);
  delay(5000);
  }
*/

/*
  void runMotor_L(int spd, int dir) {
  int dir_1;
  int dir_2;

  if (dir) {
    //clockwise
    dir_1 = LOW;
    dir_2 = HIGH;
  } else if (!dir) {
    //anticlockwise
    dir_1 = HIGH;
    dir_2 = LOW;
  }

  digitalWrite(LIN1, dir_1);
  digitalWrite(LIN2, dir_2);
  analogWrite(PWML, spd);
  }

  void runMotor_R(int spd, int dir) {
  int dir_1;
  int dir_2;

  if (dir) {
    //clockwise
    dir_1 = LOW;
    dir_2 = HIGH;
  } else if (!dir) {
    //anticlockwise
    dir_1 = HIGH;
    dir_2 = LOW;
  }

  digitalWrite(RIN1, dir_1);
  digitalWrite(RIN2, dir_2);
  analogWrite(PWMR, spd);
  }
*/

void runMotor_L(int spd) {
  int dir_1 = LOW;
  int dir_2 = HIGH;

  digitalWrite(LIN1, dir_1);
  digitalWrite(LIN2, dir_2);
  analogWrite(PWML, spd);
}

void runMotor_R(int spd) {
  int dir_1 = LOW;
  int dir_2 = HIGH;

  digitalWrite(RIN1, dir_1);
  digitalWrite(RIN2, dir_2);
  analogWrite(PWMR, spd);
}
