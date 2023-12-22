#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <HttpClient.h>
#include <WiFiMulti.h>

#include "wifi_config.h"

// netcat client
// nc -l 8000 first then you can control it
WiFiClient client;

int status = WL_IDLE_STATUS;

struct LedState
{
  char name[64];
  int port; 
  int led;
  char line[128];

  char ip[64];
  int netcat_port;
  int html_port;

};

LedState send_object;

void postDataToServer();

void setup() {

  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.println("Attempting to connect to wifi.");

  status = WiFi.begin(WIFI_SSID, WIFI_PWD);
  while( WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Attempting to connect to wifi.");
    delay(1000);
  }

  Serial.println("Success! Connect to your wifi network.");

  strlcpy(send_object.name,"bilbo",sizeof(send_object.name) );
  send_object.led = 1;
  send_object.port = 8000;
  send_object.netcat_port = 8000;
  send_object.html_port = 5000;

  strlcpy(send_object.ip,WiFi.localIP().toString().c_str(), sizeof(send_object.ip));

  Serial.print("Trying to connect to server: ");
  Serial.print(send_object.name);
  Serial.print(":");
  Serial.println(send_object.port);
  if( client.connect(send_object.name, send_object.port))
  {
    Serial.println("Connected.");

    client.println("Esp32 is Connected");
    client.print("> ");
  }


  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  postDataToServer();

}

void updateLed(int on)
{
  digitalWrite(LED_BUILTIN,on);
  send_object.led = on;
}

void postDataToServer()
{

  HTTPClient http;
  http.begin("http://bilbo:5000/service");
  http.addHeader("Content-Type","application/json");

  StaticJsonDocument<512> doc;
  doc["name"] = send_object.name;
  doc["led"] = send_object.led;
  doc["port"] = send_object.port;
  doc["ip"] = send_object.ip;
  doc["netcat_port"] = send_object.netcat_port;
  doc["html_port"] = send_object.html_port;

  String requestBody;
  serializeJson(doc, requestBody);

  int httpResponseCode = http.POST(requestBody);

  // Serial.print("Http Response Code : ");
  // Serial.println(httpResponseCode);

  if(httpResponseCode > 0)
  {
    StaticJsonDocument<512> req;
    String command = http.getString();
    // Serial.print("Command Received: ");
    // Serial.println(command);
    deserializeJson(req, command);
    updateLed(req["led"]);
  }

}

void processLine(String* line)
{
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc,*line);
  if(error)
  {
    Serial.println(F("Failed to parse line"));
    return;
  }

  send_object.led = doc["led"];
  digitalWrite(LED_BUILTIN,send_object.led);
  Serial.print("led updated: ");
  Serial.println(send_object.led);
}

void generate()
{
    StaticJsonDocument<512> doc;
    doc["name"] = send_object.name;
    doc["led"] = send_object.led;
    doc["port"] = send_object.port;

    Serial.println("Waiting for client string...");
    String line = client.readStringUntil('\n');

    processLine(&line);

    client.print("Received: ");
    line.replace("dad","stupid");

    strlcpy(send_object.line,
            line.c_str(),
            sizeof(send_object.line));
    doc["line"] = send_object.line;
    doc["led"] = send_object.led;

    String output;
    if( serializeJsonPretty(doc,output) == 0 )
    {
      Serial.println(F("Failed to serialize string"));
    }
    else
    {
      client.println(output.c_str());
      client.print("> ");
    }

    postDataToServer();

}

long last_update = 0;

void loop() {

  // put your main code here, to run repeatedly:
  // if there are incoming bytes available
  // from the server, read them and print them:
  if (client.available()) {
    generate();
  }

  if( (millis() - last_update) > 100 )
  {
    postDataToServer();
    last_update = millis();
  }

}
