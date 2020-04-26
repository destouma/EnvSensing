#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP280.h>

#include "arduino_secrets.h" 

// your network SSID (name)
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
// your network key Index number (needed only for WEP)   
int keyIndex = 0;            

int status = WL_IDLE_STATUS;

// Web server informations
IPAddress server(192,168,1,204);
int port = 80;
WiFiClient client;
HttpClient client1 = HttpClient(client, server, port);
String response;
int statusCode = 0;

// last time you connected to the server, in milliseconds
unsigned long lastConnectionTime = 0;  
// delay between updates, in milliseconds             
const unsigned long postingInterval = 600L * 1000L; 


// BMP 280 sensor
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp;

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
  // if 600 seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
  }
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  lastConnectionTime = millis();
  // Post request with JSON content in a string
  Serial.println("making POST request");
//  String contentType = "application/json";
  String postData = getPostDataJson(getTemperature(),getPressure(),getBatteryLevel());
//  Serial.println("JSON content:" + postData);
//  client1.post("/api/v1/device_readings.json", contentType,postData);
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

String getPostDataJson(long temp, long pres, long batt){
  StaticJsonDocument<200> doc;
  doc["device_uuid"] = "123-123-000-000";
  JsonArray readings = doc.createNestedArray("sensor_readings");
  JsonObject object1 = readings.createNestedObject();
  object1["sensor_uuid"] = "123-123-000-001";
  object1["sensor_value"] = temp;
  JsonObject object2 = readings.createNestedObject();
  object2["sensor_uuid"] = "123-123-000-002";
  object2["sensor_value"] = pres;
  JsonObject object3 = readings.createNestedObject();
  object3["sensor_uuid"] = "123-123-000-003";
  object3["sensor_value"] = batt;

  String tmpJson;
  serializeJson(doc, tmpJson);
  Serial.print("Json: ");
  Serial.println(tmpJson);
  return tmpJson;
}

int getBatteryLevel(){
  int battery = analogRead(ADC_BATTERY);
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  
  Serial.print("Battery Level % is now: "); // print it
  Serial.println(batteryLevel);

  return batteryLevel;
}

long getTemperature(){
  short temperature = bmp.readTemperature() * 100;
  Serial.print(F("Temperature = "));
  Serial.println(temperature);
  
  return temperature;
}

long getPressure(){
 unsigned long pressure = bmp.readPressure(); 
 Serial.print(F("Pressure = "));
 Serial.println(pressure);
 return pressure;
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
