#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>
#include "cactus_io_BME280_I2C.h"


const int ledPin = LED_BUILTIN; // set ledPin to on-board LED

// Bluetooth
BLEService environmentalSensingService("181A");
BLEUnsignedLongCharacteristic pressureChar("2A6D", BLERead | BLENotify );
BLEShortCharacteristic temperatureChar("2A6E", BLERead | BLENotify );
BLEShortCharacteristic humidityChar("2A6F", BLERead | BLENotify );
BLEDescriptor configDesc("2902", "config");

BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",BLERead | BLENotify);


// BME 280
BME280_I2C bme(0x76); 


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);    // initialize serial communication
  while (!Serial);
  
  // BME280 sensor
  if (!bme.begin()) {
    #ifdef DEBUG
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
    #endif
    while (1);
  }

  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }
 

  BLE.setDeviceName("ARDUINO");
  BLE.setLocalName("ARDUINO");
  
  BLE.setAdvertisedService(environmentalSensingService); 
  environmentalSensingService.addCharacteristic(pressureChar); 
  environmentalSensingService.addCharacteristic(temperatureChar); 
  environmentalSensingService.addCharacteristic(humidityChar); 
  BLE.addService(environmentalSensingService); 

//  batteryService.addCharacteristic(batteryLevelChar);
//  BLE.addService(batteryService);

//  ledService.addCharacteristic(ledCharacteristic);
//  BLE.addService(ledService);

  pressureChar.setValue(0);
  temperatureChar.setValue(0);
  batteryLevelChar.setValue(0);
  
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    Serial.println();

    updateCharValues();
    Serial.println();
  }
  
}

void updateCharValues(){
  bme.readSensor();

  float pressure = bme.getPressure_MB(); 
  float temperature = bme.getTemperature_C();
  float humidity = bme.getHumidity();

  int iPressure = (int) (pressure * 10000);
  int iTemperature = (int) (temperature * 100);
  int iHumidity = (int) (humidity * 100);
  



  Serial.print(F("Temperature = "));
  Serial.println(temperature);

  Serial.print(F("Pressure = "));
  Serial.println(pressure);
  
  Serial.print(F("Humidity = "));
  Serial.println(humidity);
}

void updateBatteryLevel() {
  int battery = analogRead(ADC_BATTERY);
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  
  batteryLevelChar.writeValue(batteryLevel);
  Serial.print("Battery Level % is now: "); // print it
  Serial.println(batteryLevel);
}
