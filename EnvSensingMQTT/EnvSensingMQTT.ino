#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP280.h>
#include "arduino_secret.h" 

// your network SSID (name)
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
// your network key Index number (needed only for WEP)   
int keyIndex = 0;            

int status = WL_IDLE_STATUS;

// Web server informations
IPAddress server(192,168,1,238);
int port = 80;
WiFiClient client;
HttpClient client1 = HttpClient(client, server, port);
String response;
int statusCode = 0;       
String postData;
int nbLoop =0;

// BMP 280 sensor
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
Adafruit_BMP280 bmp;
int battery=0;
int batteryLevel=0;
short temperature=0;
unsigned long pressure=0;
unsigned long counter=0;
void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
      // wait for serial port to connect. Needed for native USB port only
    ; 
  }

  // check bmp280 sensor
  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

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
//  updateBatteryLevel();
//  updateTemperature();
//  updatePressure();
//  httpRequest();
//  delay(10000);
}

// this method makes a HTTP connection to the server:
void httpRequest() {
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
  client1.sendHeader("Host", "192.168.1.204");
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

void updateBatteryLevel(){
  battery = analogRead(ADC_BATTERY);
  batteryLevel = map(battery, 0, 1023, 0, 100);
  Serial.print("Battery Level % is now: "); // print it
  Serial.println(batteryLevel);
}

void updateTemperature(){
  temperature = bmp.readTemperature() * 100;
  Serial.print(F("Temperature = "));
  Serial.println(temperature);
}

void updatePressure(){
 pressure = bmp.readPressure(); 
 Serial.print(F("Pressure = "));
 Serial.println(pressure);
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
