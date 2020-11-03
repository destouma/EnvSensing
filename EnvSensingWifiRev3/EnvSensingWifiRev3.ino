#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h" 

// your network SSID (name)
char ssid[] = "manu-laet";        // your network SSID (name)
char pass[] = "06071972";    // your network password (use for WPA, or use as key for WEP)
// your network key Index number (needed only for WEP)   
int keyIndex = 0;            

int status = WL_IDLE_STATUS;

// Web server informations
IPAddress server(192,168,1,215);
int port = 3000;
WiFiClient client;
HttpClient client1 = HttpClient(client, server, port);
String response;
int statusCode = 0;       
String postData;
int nbLoop =0;

// BME 280 sensor
int battery=0;
int batteryLevel=0;
short temperature=0;
unsigned long pressure=0;
unsigned long counter=0;

#define FIVEMIN (1000UL * 60 * 1)
unsigned long rolltime = millis() + FIVEMIN;

void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
      // wait for serial port to connect. Needed for native USB port only
    ; 
  }

  // check bme280 sensor
  
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

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  // you're connected now, so print out the status:
  printWifiStatus();
}


void loop() {
  if((long)(millis() - rolltime) >= 0) {
    getCurrentDateTime();
    getAllSensors("123-123-000-000");
    rolltime += FIVEMIN;
  }
}


void getCurrentDateTime() {
  client1.get("/api/v1/date_time/current_date_time.json");

  statusCode = client1.responseStatusCode();
  response = client1.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);   
}

void getAllSensors(String device_uuid) {
  client1.get("/api/v1/sensors.json?device_uuid=" + device_uuid);

  statusCode = client1.responseStatusCode();
  response = client1.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
    
}

void postReadingsRequest() {
  Serial.println("making POST request");
  postData="";
  counter++;
  StaticJsonDocument<500> doc;
  doc["device_uuid"] = "123-123-000-000";
  JsonArray readings = doc.createNestedArray("sensor_readings");
  JsonObject object1 = readings.createNestedObject();
  object1["sensor_uuid"] = "123-123-000-001";
  object1["sensor_value"] = temperature;
  JsonObject object2 = readings.createNestedObject();
  object2["sensor_uuid"] = "123-123-000-002";
  object2["sensor_value"] = pressure;
  JsonObject object3 = readings.createNestedObject();
  object3["sensor_uuid"] = "123-123-000-003";
  object3["sensor_value"] = batteryLevel;
  JsonObject object4 = readings.createNestedObject();
  object4["sensor_uuid"] = "123-123-000-004";
  object4["sensor_value"] = counter;
  serializeJson(doc, postData);
  
  Serial.print("Post Data:");
  Serial.println(postData);

  client1.beginRequest();
  client1.post("/api/v1/device_readings.json");
  client1.sendHeader("Host", "192.168.1.238");
  client1.sendHeader(HTTP_HEADER_CONTENT_TYPE, "application/json");
  client1.sendHeader(HTTP_HEADER_CONTENT_LENGTH, postData.length());
  client1.endRequest();
  client1.write((const byte*)postData.c_str(), postData.length());

  // read the status code and body of the response
  statusCode = client1.responseStatusCode();
  response = client1.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
