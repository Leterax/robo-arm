/*
  To upload through terminal you can use: curl -F "image=@firmware.bin" esp32-webupdate.local/update
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPUpdateServer.h>
#include <ESP32Servo.h>


#ifndef STASSID
#define STASSID "roboarm"
// #define STAPSK "tn1337pw!"
#endif

const char* host = "esp32-webupdate";
const char* ssid = STASSID;
// const char* password = STAPSK;

WebServer httpServer(80);
HTTPUpdateServer httpUpdater;


Servo base;
Servo link1;
Servo link2;
Servo link3;
Servo head1;
Servo head2;


Servo servos[6];

float goalAngles[6];
float currentAngles[6];
float lastAngles[6];

unsigned long lastMillis;

float easeInOutQuad(float x) {
  return x < 0.5 ? 2 * x * x : 1 - ((-2 * x + 2) * (-2 * x + 2)) / 2;
}


void setup(void) {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");

  WiFi.softAP(ssid);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

    // Set up the endpoint for setting the servo angles
  httpServer.on("/set-angles", []() {
    // Read the 6 angles from the query string
    for (int i = 0; i < 6; i++) {
      goalAngles[i] = httpServer.arg(i).toFloat();
    }

    // Create a JSON object to hold the angles
    char tmp[250];
    sprintf(tmp, "Angles set to %f %f %f %f %f %f\n", goalAngles[0],  goalAngles[1],  goalAngles[2],  goalAngles[3],  goalAngles[4],  goalAngles[5]);

    // Send the JSON string as the response to the client
    httpServer.send(200, "text/plain", tmp);
  });


  // servo setup
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  int pins[6]{32,33,25,26,27,12};

  for (int i=0; i<6; i++) {
    servos[i].setPeriodHertz(300);
    servos[i].attach(pins[i], 500, 3000);
    servos[i].write(90);
    currentAngles[i] = 90;
    goalAngles[i] = 90;
  }
}

int i = 0;

void loop(void) {
  httpServer.handleClient();
  
  if (millis() - lastMillis >= 5) {
    lastMillis = millis();
    for (int i=0;i<6;i++) {
      currentAngles[i] = currentAngles[i] * 0.95 + goalAngles[i] * 0.05;
      servos[i].write(int(currentAngles[i]+0.5));   
    }
  }


}