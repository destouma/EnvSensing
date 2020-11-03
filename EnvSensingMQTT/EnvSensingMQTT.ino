#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>
#include <String.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "arduino_secret.h" 

// MQTT
#define BROKER_IP    "192.168.1.238"
#define DEV_NAME     "mqttdevice"
#define MQTT_USER    ""
#define MQTT_PW      ""

// BMP 280
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

// WIFI
#define SECRET_SSID "manu-laet"
#define SECRET_PASS "06071972"

// WIFI Client
WiFiClient clientWIFI;
char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

// MQTT client 
MQTTClient clientMQTT;

// BMP 280 sensor
Adafruit_BMP280 bmp;
short temperature=0;
unsigned long pressure=0;

void setup() {
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
//  while (!Serial) {
//      // wait for serial port to connect. Needed for native USB port only
//    ; 
//  }

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

  // Connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(60000);
  }
  printWifiStatus();

 // Connect to MQTT broker
 clientMQTT.begin(BROKER_IP, 1883, clientWIFI);
 clientMQTT.onMessage(messageReceived);

 Serial.println("Attempting to connect to MQTT broker");
 while (!clientMQTT.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
   Serial.println("Attempting to connect to MQTT broker");
   delay(1000);
 }
 Serial.println("connected!");
 clientMQTT.subscribe("temperature"); 
 clientMQTT.subscribe("pressure"); 

}

void loop() {
  publishTemperature();
  publishPressure();
  delay(10000);
}


void publishTemperature(){
  temperature = bmp.readTemperature() * 100;
  Serial.print(F("Temperature = "));
  Serial.println(temperature);
  clientMQTT.publish("temperature", String(temperature)); 
}

void publishPressure(){
 pressure = bmp.readPressure(); 
 Serial.print(F("Pressure = "));
 Serial.println(pressure);
 
 clientMQTT.publish("pressure", String(pressure)); 
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

void messageReceived(String &topic, String &payload) {
 Serial.println("incoming: " + topic + " - " + payload);
}
