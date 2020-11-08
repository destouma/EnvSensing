#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 
#include "cactus_io_BME280_I2C.h"
#include <ArduinoLowPower.h>
#include <CooperativeMultitasking.h>

char ssid[] = SECRET_SSID;       
char pass[] = SECRET_PASS;   

int status = WL_IDLE_STATUS;

// Web server informations
IPAddress server(SERVER_IP_1,SERVER_IP_2,SERVER_IP_3,SERVER_IP_4);
int port = SERVER_PORT;
WiFiClient client;
HttpClient client1 = HttpClient(client, server, port);

// BME 280
BME280_I2C bme(0x76); // I2C using address 0x76

CooperativeMultitasking tasks;

void setup() {
  //SERIAL init
  Serial.begin(9600);
  while (!Serial) {
    delay(1000); 
  }

  // BME280 sensor
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  bme.setTempCal(-1);
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // Check WiFi firmware
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  WiFi.begin(ssid, pass);
  tasks.after(10000, checkWiFi); // after 10 seconds call checkWiFi()
  tasks.after(30000, postData);
}

void loop() {
   tasks.run();
}

void checkWiFi() {
  switch (WiFi.status()) {
    case WL_CONNECT_FAILED:
    case WL_CONNECTION_LOST:
    case WL_DISCONNECTED:
      Serial.println("wifi not connected");
      WiFi.begin(ssid, pass);
      tasks.after(10000, checkWiFi);
      return;
  }
  tasks.after(30000, checkWiFi); 
}


void postData(){
  bme.readSensor();
  Serial.println();
  Serial.print(bme.getPressure_MB()); Serial.print(" mb\t"); // Pressure in millibars
  Serial.print(bme.getHumidity()); Serial.print(" %\t\t");
  Serial.print(bme.getTemperature_C()); Serial.print(" *C\t");
  Serial.print(bme.getTemperature_F()); Serial.println(" *F\t");
       
  // Post temperature
  postReadingRequest("123-123-000-001",bme.getTemperature_C()*100);
  // Post Pressure
  postReadingRequest("123-123-000-002",bme.getPressure_MB()*10000);
  // Post Humidity
  postReadingRequest("123-123-000-003",bme.getHumidity()*100);
  
  tasks.after(30000, postData);
}

void postReadingRequest(String uuid, int value) { 
  String response;
  int statusCode = 0;
  
  String postData = "";
  StaticJsonDocument<500> doc;
  doc["sensor_uuid"] = uuid;
  doc["sensor_value"] = value;
  serializeJson(doc, postData);
  
  Serial.print("Post Data     : ");
  Serial.println(postData);

  client1.beginRequest();
  client1.post("/api/v1/sensor_readings.json");
  client1.sendHeader("Host", "192.168.1.238");
  client1.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/json");
  client1.sendHeader(HTTP_HEADER_CONTENT_LENGTH, postData.length());
  client1.endRequest();
  client1.write((const byte*)postData.c_str(), postData.length());

  // read the status code and body of the response
  statusCode = client1.responseStatusCode();
  response = client1.responseBody();
  Serial.print("   status code: ");
  Serial.println(statusCode);
  Serial.print("   response   : ");
  Serial.println(response);
}
