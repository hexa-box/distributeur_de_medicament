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
//Lib servomotor
#include <ESP32Servo.h>
//--------------------------------------------------------------------------------------------------
//Lib time from NTP server
#include "time.h"
//--------------------------------------------------------------------------------------------------
//Lib bluetooth communication
#include "BluetoothSerial.h"
//--------------------------------------------------------------------------------------------------
// Variable bluetooth
BluetoothSerial SerialBT;

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

const int STEEPER_SPEED = 5; // Speed at 5 rpm

Stepper STEEPER(STEPS_PER_REVOLUTION, IN1, IN3, IN2, IN4);

int askedSteps = 0;
//--------------------------------------------------------------------------------------------------
//servomotor 
/*
ESP32       Couleur du fil SG90
GND         Marron
5V ou 3V3   Rouge
GPIO22      Orange
*/

const int PIN_SG90 = 22; 
Servo SERVOMOTOR;
int askedPosition = -1;
//--------------------------------------------------------------------------------------------------
const char* NTP_SERVER = "pool.ntp.org";  //server ntp you want to use. 
const long  GMT_OFFSET_SEC = 0;           //variable defines the offset in seconds between your time zone and GMT, for Portugalthe time offset is 0 
const int   DAY_LIGHT_OFFSET_SEC = 3600;  // variable defines the offset in seconds for daylight saving time. It is generally one hour, that corresponds to 3600 seconds

//--------------------------------------------------------------------------------------------------

void initNtpTime()
{
  // Init and get the time
  configTime(GMT_OFFSET_SEC, DAY_LIGHT_OFFSET_SEC, NTP_SERVER);
}


//--------------------------------------------------------------------------------------------------
// init Bluetooth
void iniBluetooth() {
  SerialBT.begin("ESP32test"); //Bluetooth device name
}

//--------------------------------------------------------------------------------------------------

void initServomotor() {
  SERVOMOTOR.setPeriodHertz(50); // Fréquence PWM pour le SG90
  SERVOMOTOR.attach(PIN_SG90, 500, 2400); // Largeur minimale et maximale de l'impulsion (en µs) pour aller de 0° à 180°
}

void moveServoTo(int position){
  SERVOMOTOR.write(position);
  Serial.print("Nombre de pas: ");
  Serial.println(position);
}
    
//--------------------------------------------------------------------------------------------------


// initialize the stepper library
void initStepper() 
{
  // set the speed at 5 rpm
  STEEPER.setSpeed(STEEPER_SPEED);
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

struct tm getNtpTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }
 
  return timeinfo;
}
//--------------------------------------------------------------------------------------------------
void setup()
{
  // initialize the serial port
  Serial.begin(9600);

  // Stepper init 
  initStepper();

  //servomotor init 
  initServomotor();

  //Bluetooth init 
  iniBluetooth();

  // Init server Wifi  
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
  }

  // Affichage de l'addresse IP 
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  //Init NTP Time
  initNtpTime();
  
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) 
  {
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


  server.on("/lock", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<100> data;
    if (request->hasParam("value"))
    {
      String value = request->getParam("value")->value();
      if(value == "true")
      {
        askedPosition = 180;
        data["message"] = "lock is true";
      }
      else if(value == "false")
      {
        askedPosition =  0;
        data["message"] = "lock is false";
      }
    }
    else
    {
      data["message"] = "No value parameter";
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

  server.addHandler(handler);
  server.onNotFound(notFound);
  server.begin();
}

void loop()
{
  if(askedSteps != 0){
    moveStepperTo(askedSteps);
    askedSteps = 0;
  }

  if(askedPosition != -1){
    moveServoTo(askedPosition);
    askedPosition = -1;
  }

  // ntp server part 
  struct tm timeinfo = getNtpTime();
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  

  // Bluetooth part 
  if (Serial.available()) {
    SerialBT.write(Serial.read());
  }
  if (SerialBT.available()) {
    Serial.write(SerialBT.read());
  }

  delay(1000);
}


/*
  Ressources:
  https://raphaelpralat.medium.com/example-of-json-rest-api-for-esp32-4a5f64774a05
  https://randomnerdtutorials.com/esp32-bluetooth-classic-arduino-ide/
  https://randomnerdtutorials.com/install-esp32-filesystem-uploader-arduino-ide/
  https://randomnerdtutorials.com/esp32-flash-memory/
  https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
  https://www.youtube.com/watch?v=VJaOULvMxeM
  
*/