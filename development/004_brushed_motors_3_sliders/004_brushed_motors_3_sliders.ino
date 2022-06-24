#include <string>
#include <ESP32Servo.h>
#include <WiFi.h>

// assign output variables to GPIO pins
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

IPAddress ip;

// replace these with your network credentials!
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* pass = "REPLACE_WITH_YOUR_PASSWORD";

// set the web server's port number to port 80
WiFiServer server(80);
int status = WL_IDLE_STATUS;
// create servo objects to control the 3-phase BRUSHLESS motors
Servo motor_H;

// auxiliar variables to store the current output state
int pos = 0;
// offset variables for calibration for the speed controller
int XC10A_offset = 1470;
int slider_value;

void setup() {
  Serial.begin(115200);

  motor_H.setPeriodHertz(50);
  motor_H.attach(SERVO_PIN_H, 500, 2400);
  
  //set GPIO pins as OUTPUTS for LEFT MOTOR
  pinMode(LIN1, OUTPUT);
  pinMode(LIN2, OUTPUT);
  pinMode(PWML, OUTPUT);
  //set GPIO pins as OUTPUTS for RIGHT MOTOR
  pinMode(RIN1, OUTPUT);
  pinMode(RIN2, OUTPUT);
  pinMode(PWMR, OUTPUT);

  // attempt to connect to the WiFi network
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);

    // connect to a WPA/WPA2 network. change this line if using open or WEP network
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection
    delay(10000);
  }

  // sets 3-phase BRUSHLESS motor to zero speed
  motor_H.writeMicroseconds(XC10A_offset);
  // sets BRUSHED motors to zero speed
  runMotor_L(0);
  runMotor_R(0);

  server.begin();

  // printing the SSID of the network the ESP32 is connected to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // printing the WiFi shield's/ESP32's IP address
  ip = WiFi.localIP();
  Serial.print("IP address: ");
  Serial.println(ip);

  // printing the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  // printing the address to view the web page from in a browser
  Serial.println("To view this page in action, please connect to the web server by visiting ");
  Serial.print("http://");
  Serial.println(ip);
}

void loop() {
  // listens for incoming clients
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("New client connected.");
  }
  if (client)
  {
    String current_line = "";
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();

        if (c == '\n')
        {
          // if the current line is blank, two newline characters are received consecutively
          // this is the end of the client HTTP request, so send a response
          if (current_line.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)...
            // and a content-type so the client knows what to expect/receive, followed by a blank line
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println();

            client.println("\
              <!DOCTYPE html>\
              <html>\
              <head>\
                  <meta name='viewport' content='width=device-width, initial-scale=1'>\
                  <link rel='icon' href='data:,'>\
                  <style>\
                      html {\
                          font-family: Helvetica;\
                          display: inline-block;\
                          margin: 0px auto;\
                          text-align: center;\
                      }\
                      .slidecontainer {\
                          width: 100%;\
                      }\
                      .slider {\
                          -webkit-appearance: none;\
                          appearance: none;\
                          width: 100%;\
                          height: 15px;\
                          margin-bottom: 20px;\
                          background: #d3d3d3;\
                          outline: none;\
                          opacity: 0.7;\
                          -webkit-transition: .2s;\
                          transition: opacity .2s;\
                      }\
                      .slider::-webkit-slider-thumb {\
                          -webkit-appearance: none;\
                          appearance: none;\
                          width: 25px;\
                          height: 25px;\
                          border-radius: 50%;\
                          background: #04AA6D;\
                          cursor: pointer;\
                      }\
                      .slider::-moz-range-thumb {\
                          width: 25px;\
                          height: 25px;\
                          border-radius: 50%;\
                          background: #04AA6D;\
                          cursor: pointer;\
                      }\
                      .slider:hover {\
                          opacity: 1;\
                      }\
                  </style>\
              </head>\
              <body>\
                  <h1>ESP32 Web Server</h1>\
                  <p>Drag the slider to change the position of the servo.</p>\
                  <div class='slidecontainer'>\
                      <p>Hover Control: <span id='hoverValueDisplay'></span></p>\
                      <input type='range' min='0' max='250' value='0' class='slider' id='hoverSlider' style='color: #00FF00'>\
                  </div>\
                  <div class='slidecontainer'>\
                      <p>Left Motor: <span id='lValueDisplay'></span></p>\
                      <input type='range' min='0' max='255' value='0' class='slider' id='lSlider' style='color: #FF0000'>\
                  </div>\
                  <div class='slidecontainer'>\
                      <p>Right Motor: <span id='rValueDisplay'></span></p>\
                      <input type='range' min='0' max='255' value='0' class='slider' id='rSlider' style='color: #0000FF'>\
                  </div>\
                  <script>\
                      let hoverSlider = document.getElementById('hoverSlider');\
                      let hoverValueDisplay = document.getElementById('hoverValueDisplay');\
                      let lSlider = document.getElementById('lSlider');\
                      let lValueDisplay = document.getElementById('lValueDisplay');\
                      let rSlider = document.getElementById('rSlider');\
                      let rValueDisplay = document.getElementById('rValueDisplay');\
                      hoverValueDisplay.innerHTML = hoverSlider.value;\
                      lValueDisplay.innerHTML = lSlider.value;\
                      rValueDisplay.innerHTML = rSlider.value;\
                      \
                      let lpad = (value, padding) => {\
                          let zeroes = new Array(padding + 1).join('0');\
                          return (zeroes + value).slice(-padding);\
                      };\
                      \
                      hoverSlider.oninput = function() {\
                          hoverValueDisplay.innerHTML = this.value;\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/H' + lpad(this.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                      lSlider.oninput = function() {\
                          lValueDisplay.innerHTML = this.value;\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/L' + lpad(this.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                      rSlider.oninput = function() {\
                          rValueDisplay.innerHTML = this.value;\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/R' + lpad(this.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                  </script>\
              </body>\
              </html>\
            ");

            // the HTTP response ends with the following blank line
            client.println();

            // break out of the WHILE loop
            break;
          }
          else
          {
            // if a newline is received, clear the current_line
            current_line = "";
          }
        }
        else if (c != '\r')
        {
          // if anything else is received which is NOT a carriage return character...
          current_line += c;
        }

        // check and verify if the client's request was to change the state of the servo/motor speed
        // the request should be in the format of GET /H000, GET /L000, GET /R000
        int curr_line_length = current_line.length();
        int get_req_beg_index = 0;
        int get_req_end_index = 5;
        if (curr_line_length == 9)
        {
          String poss_get_req = current_line.substring(get_req_beg_index, get_req_end_index);
          String poss_slider_param = current_line.substring(get_req_end_index, get_req_end_index + 1);
          String poss_slider_value = current_line.substring(get_req_end_index + 1, curr_line_length);
          if (poss_get_req == "GET /")
          {
            auto is_number = [](String s)
            {
              for (char c : s)
              {
                if (c == ' ' || c == '\n' || c == '\t') continue;
                if (std::isdigit(c) == 0) return false;
              }
              return true;
            };
            if (is_number(poss_slider_value))
            {
              Serial.println(poss_get_req + poss_slider_param + poss_slider_value);
              slider_value = atoi(poss_slider_value.c_str());

              if (poss_slider_param[0] == 'H')
              {
                motor_H.writeMicroseconds((slider_value * 2) + XC10A_offset);
              }
              else if (poss_slider_param[0] == 'L')
              {
                runMotor_L(slider_value);
              }
              else if (poss_slider_param[0] == 'R')
              {
                runMotor_R(slider_value);
              }
              Serial.println("Updated servo motor: " + poss_slider_param + slider_value);
            }
          }
        }
      }
    }
  }
}

void runMotor_L(int spd) {
  int dir_1 = HIGH;
  int dir_2 = LOW;

  digitalWrite(LIN1, dir_1);
  digitalWrite(LIN2, dir_2);
  analogWrite(PWML, spd);
}

void runMotor_R(int spd) {
  int dir_1 = HIGH;
  int dir_2 = LOW;

  digitalWrite(RIN1, dir_1);
  digitalWrite(RIN2, dir_2);
  analogWrite(PWMR, spd);
}
