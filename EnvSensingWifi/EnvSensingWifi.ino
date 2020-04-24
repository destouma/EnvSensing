#include <SPI.h>
#include <WiFiNINA.h>
#include <Wire.h>
#include <HttpClient.h>

#include "arduino_secrets.h" 

// your network SSID (name)
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
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

// last time you connected to the server, in milliseconds
unsigned long lastConnectionTime = 0;  
// delay between updates, in milliseconds             
const unsigned long postingInterval = 10L * 1000L; 


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
      // wait for serial port to connect. Needed for native USB port only
    ; 
  }


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
  String contentType = "application/json";
  String postData = "{\"sensor_uuid\":\"123-123-000-002\",\"sensor_value\":102300}";

  client1.post("/api/v1/sensor_readings", contentType, postData);

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
