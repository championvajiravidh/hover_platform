#include <string>
#include <ESP32Servo.h>
#include <WiFi.h>

// assign output variables to GPIO pins
// initialise PINs on ESP32 to servoMotor
#define HOVER_MOTOR_PIN 14
#define LEFT_MOTOR_PIN 15
#define RIGHT_MOTOR_PIN 22
#define SERVO_PIN_L 5
#define SERVO_PIN_R 21

IPAddress ip;

// replace these with your network credentials!
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* pass = "REPLACE_WITH_YOUR_PASSWORD";

// set the web server's port number to port 80
WiFiServer server(80);
int status = WL_IDLE_STATUS;
// Create servo objects to control the servo and 3-phase BRUSHLESS motors
Servo motor_H;
Servo motor_L;
Servo motor_R;
Servo servo_L;
Servo servo_R;

// auxiliar variables to store the current output state
int pos = 0;
// offset variables for calibration for each speed controller/servo
int XC10A_offset = 1470;
int esc_offset = 1000;
int servo_offset = 45;
int slider_value;
int h_value;
int prop = 0.7;

void setup() {
  Serial.begin(115200);

  motor_H.setPeriodHertz(50);
  motor_H.attach(HOVER_MOTOR_PIN, 500, 2400);
  motor_L.setPeriodHertz(50);
  motor_L.attach(LEFT_MOTOR_PIN, 500, 2400);
  motor_R.setPeriodHertz(50);
  motor_R.attach(RIGHT_MOTOR_PIN, 500, 2400);

  servo_L.setPeriodHertz(50); // standard 50 hertz servo motor
  servo_L.attach(SERVO_PIN_L, 500, 2400); // attaches the servo on PIN 21 of the ESP to the servo object
  servo_R.setPeriodHertz(50);
  servo_R.attach(SERVO_PIN_R, 500, 2400);

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

  // sets 3-phase BRUSHLESS motors to zero speeds
  motor_H.writeMicroseconds(XC10A_offset);
  motor_L.writeMicroseconds(esc_offset);
  motor_R.writeMicroseconds(esc_offset);
  // sets servos to neutral positions
  servo_L.write(90);
  servo_R.write(90);

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
    Serial.println("New client connected!");
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
                  <link rel='stylesheet' href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css'>\
                  <style>\
                      html {\
                          font-family: Courier;\
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
                      button {\
                          background-color: DodgerBlue;\
                          border: none;\
                          color: white;\
                          padding: 12px 16px;\
                          font-size: 16px;\
                          cursor: pointer;\
                      }\
                      button:hover {\
                          background-color: RoyalBlue;\
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
                  <button id='forwardsButton'><i class='fa-solid fa-arrow-up'></i></button>\
                  <div class='slidecontainer'>\
                      <p> Power : <span id='powerValueDisplay'></span></p>\
                      <input type='range' min='0' max='500' value='0' class='slider' id='powerSlider' style='color: #FF0000'>\
                  </div>\
                  <div class='slidecontainer'>\
                      <p> Direction : <span id='directionValueDisplay'></span></p>\
                      <input type='range' min='0' max='90' value='45' class='slider' id='directionSlider' style='color: #FF0000'>\
                  </div>\
                  <button id='stopButton'><i class='fa-solid fa-stop'></i></button>\
                  <script>\
                      let hoverSlider = document.getElementById('hoverSlider');\
                      let hoverValueDisplay = document.getElementById('hoverValueDisplay');\
                      let powerSlider = document.getElementById('powerSlider');\
                      let powerValueDisplay = document.getElementById('powerValueDisplay');\
                      let directionSlider = document.getElementById('directionSlider');\
                      let directionValueDisplay = document.getElementById('directionValueDisplay');\
                      hoverValueDisplay.innerHTML = hoverSlider.value;\
                      powerValueDisplay.innerHTML = powerSlider.value;\
                      directionValueDisplay.innerHTML = directionSlider.value;\
                      \
                      let forwardsButton = document.getElementById('forwardsButton');\
                      let stopButton = document.getElementById('stopButton');\
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
                      powerSlider.oninput = function() {\
                          powerValueDisplay.innerHTML = this.value;\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/P' + lpad(this.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                      directionSlider.oninput = function() {\
                          directionValueDisplay.innerHTML = this.value;\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/D' + lpad(this.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                      forwardsButton.onclick = function() {\
                          directionValueDisplay.innerHTML = '45';\
                          directionSlider.value = '45';\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/F' + lpad(directionSlider.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                      \
                      stopButton.onclick = function() {\
                          hoverValueDisplay.innerHTML = '0';\
                          hoverSlider.value = '0';\
                          powerValueDisplay.innerHTML = '0';\
                          powerSlider.value = '0';\
                          \
                          let localIP = window.location.host;\
                          let address = 'http://' + localIP + '/S' + lpad(hoverSlider.value, 3);\
                          let xmlHttp = new XMLHttpRequest();\
                          \
                          console.log('Sending XMLHttpRequest: GET ' + address);\
                          xmlHttp.open('GET', address, false);\
                          xmlHttp.send(null);\
                      };\
                      \
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
        // the request should be in the format of GET /H000, GET /L000, GET /R000, GET /S000, GET /V000
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
                h_value = slider_value * 2;
              }
              else if (poss_slider_param[0] == 'P')
              {
                motor_L.writeMicroseconds((slider_value * 2) + esc_offset);
                motor_R.writeMicroseconds((slider_value * 2) + esc_offset + (h_value * prop));
              }
              else if (poss_slider_param[0] == 'D')
              {
                servo_L.write(slider_value + servo_offset);
                servo_R.write(slider_value + servo_offset);
              }
              else if (poss_slider_param[0] == 'F')
              {
                servo_L.write(90);
                servo_R.write(90);
              }
              else if (poss_slider_param[0] == 'S')
              {
                motor_H.writeMicroseconds(XC10A_offset);
                motor_L.writeMicroseconds(esc_offset);
                motor_R.writeMicroseconds(esc_offset);
              }
              Serial.println("Updated servo motor: " + poss_slider_param + slider_value);
            }
          }
        }
      }
    }
  }
}
