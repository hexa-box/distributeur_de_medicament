//Lib wifi server
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
//--------------------------------------------------------------------------------------------------
//Lib stepper motor
#include <Stepper.h> //AccelStepper
#include <ESP32Servo.h>
#include "soc/rtc_wdt.h"
//--------------------------------------------------------------------------------------------------

// Variables Wifi
AsyncWebServer server(80);
const char *SSID = "Freebox-44E6E2";
const char *PASSWORD = "wifi@temporaire@71";

// Variables stepper motor 
//--------------------------------------------------------------------------------------------------
const int STEPS_PER_REVOLUTION = 2048;  // change this to fit the number of steps per revolution

// ULN2003 Motor Driver Pins
const int IN1 = 19;
const int IN2 = 18;
const int IN3 = 5;
const int IN4 = 17;

static const int SERVO_PIN = 16;
Stepper STEEPER(STEPS_PER_REVOLUTION, IN1, IN3, IN2, IN4);

int askedSteps = 0;

//--------------------------------------------------------------------------------------------------


// initialize the stepper library
void initStepper() 
{
  // set the speed at 5 rpm
  STEEPER.setSpeed(5);
  // initialize the serial port
  Serial.begin(9600);
}

void moveStepperTo(int steps)
{
  STEEPER.step(steps);
  Serial.print("Nombre de pas: ");
  Serial.println(steps);
}

//--------------------------------------------------------------------------------------------------

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "application/json", "{\"message\":\"Not found\"}");
}

//--------------------------------------------------------------------------------------------------
void setup()
{
  // Stepper init 
  initStepper();

  // server Wifi init 
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
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
      Serial.println(request->getParam("message")->value().toInt());
      askedSteps = request->getParam("message")->value().toInt();
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
}

void loop()
{
  if(askedSteps != 0){
    moveStepperTo(askedSteps);
    askedSteps = 0;
  }
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