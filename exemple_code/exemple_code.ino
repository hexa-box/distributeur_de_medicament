#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
AsyncWebServer server(80);
const char *ssid = "Freebox-44E6E2";
const char *password = "wifi@temporaire@71";
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}
void setup()
{
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
  }
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", "{\"message\":\"Welcome\"}");
  });
  server.on("/get-message", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<100> data;
    if (request->hasParam("message"))
    {
      data["message"] = request->getParam("message")->value();
    }
    else
    {
      data["message"] = "No message parameter";
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
  });
  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/post-message", [](AsyncWebServerRequest *request, JsonVariant &json) {
    StaticJsonDocument<200> data;
    if (json.is<JsonArray>())
    {
      data = json.as<JsonArray>();
    }
    else if (json.is<JsonObject>())
    {
      data = json.as<JsonObject>();
    }
    String response;
    serializeJson(data, response);
    request->send(200, "application/json", response);
    Serial.println(response);
  });
  server.addHandler(handler);server.onNotFound(notFound);
  server.begin();
}void loop()
{
}









/*

    #include <Stepper.h> //AccelStepper
    #include <ESP32Servo.h>
    const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
    // ULN2003 Motor Driver Pins
    #define IN1 19
    #define IN2 18
    #define IN3 5
    #define IN4 17

    static const int servoPin = 16;
    Servo servo1;

    // initialize the stepper library
    Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
    void setup() {
      // set the speed at 5 rpm
      myStepper.setSpeed(5);
      // initialize the serial port
      Serial.begin(115200);
      servo1.attach(servoPin);
    }

    void loop() {
      // step one revolution in one direction:
      Serial.println("clockwise");
      myStepper.step(stepsPerRevolution);
      delay(1000);
      // step one revolution in the other direction:
      Serial.println("counterclockwise");
      myStepper.step(-stepsPerRevolution);
      delay(1000);

      // servo motor 
      for(int posDegrees = 0; posDegrees <= 180; posDegrees++) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
      }

      for(int posDegrees = 180; posDegrees >= 0; posDegrees--) {
        servo1.write(posDegrees);
        Serial.println(posDegrees);
        delay(20);
      }
    }

    */