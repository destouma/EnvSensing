#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>
#include <String.h>
#include <MQTT.h>
#include <ArduinoJson.h>
#include <Adafruit_BMP280.h>
#include "arduino_secret.h" 

#define BROKER_IP    "xxx.xxx.xxx.xxx"
#define DEV_NAME     "mqttdevice"
#define MQTT_USER    ""
#define MQTT_PW      ""
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

// your network SSID (name)
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
// your network key Index number (needed only for WEP)   
int keyIndex = 0;            

int status = WL_IDLE_STATUS;

// Web server informations
WiFiClient clientWIFI;
MQTTClient clientMQTT;
String response;
int statusCode = 0;       
String postData;
int nbLoop =0;

// BMP 280 sensor
Adafruit_BMP280 bmp;

int battery=0;
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

 clientMQTT.begin(BROKER_IP, 1883, clientWIFI);
 clientMQTT.onMessage(messageReceived);

 Serial.println("Attempting to connect to MQTT broker");
 while (!clientMQTT.connect(DEV_NAME, MQTT_USER, MQTT_PW)) {
   Serial.println("Attempting to connect to MQTT broker");
   delay(1000);
 }
 Serial.println("connected!");
 clientMQTT.subscribe("temperature"); //SUBSCRIBE TO TOPIC /hello
 clientMQTT.subscribe("pressure"); //SUBSCRIBE TO TOPIC /hello

}

void loop() {
//  updateBatteryLevel();
  updateTemperature();
  updatePressure();
//  httpRequest();
   clientMQTT.publish("pressure", String(pressure)); 
   clientMQTT.publish("temperature", String(temperature)); 
   delay(10000);
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

void messageReceived(String &topic, String &payload) {
 Serial.println("incoming: " + topic + " - " + payload);
}
