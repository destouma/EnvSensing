#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>
#include "cactus_io_BME280_I2C.h"

// Bluetooth
BLEService environmentalSensingService("181A");
BLEUnsignedLongCharacteristic pressureChar("2A6D", BLERead | BLENotify );
BLEShortCharacteristic temperatureChar("2A6E", BLERead | BLENotify );
BLEShortCharacteristic humidityChar("2A6F", BLERead | BLENotify );
BLEDescriptor configDesc("2902", "config");

BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",BLERead | BLENotify);

BME280_I2C bme(0x76); 

void setup() {
  #ifdef DEBUG
    Serial.begin(9600);    // initialize serial communication
    while (!Serial);
  #endif
  
  // BME280 sensor
  if (!bme.begin()) {
    #ifdef DEBUG
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
    #endif
    while (1);
  }

  if (!BLE.begin()) {
    #ifdef DEBUG
    Serial.println("starting BLE failed!");
    #endif

    while (1);
  }
 

  BLE.setDeviceName("ARDUINO");
  BLE.setLocalName("ARDUINO");
  
  BLE.setAdvertisedService(environmentalSensingService); 
  environmentalSensingService.addCharacteristic(pressureChar); 
  environmentalSensingService.addCharacteristic(temperatureChar); 
  environmentalSensingService.addCharacteristic(humidityChar); 
  BLE.addService(environmentalSensingService); 

  pressureChar.setValue(0);
  temperatureChar.setValue(0);
  humidityChar.setValue(0);
  
  BLE.advertise();
  #ifdef DEBUG
  Serial.println("Bluetooth device active, waiting for connections...");
  #endif
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    #ifdef DEBUG
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    Serial.println();
    #endif
    updateCharValues();
  
  }
  
}

void updateCharValues(){
  bme.readSensor();

  float pressure = bme.getPressure_MB(); 
  float temperature = bme.getTemperature_C();
  float humidity = bme.getHumidity();

  unsigned long iPressure = (int) (pressure * 10000);
  int iTemperature = (int) (temperature * 100);
  unsigned int iHumidity = (int) (humidity * 100);
  
  pressureChar.writeValue(iPressure);
  temperatureChar.writeValue(iTemperature);
  humidityChar.writeValue(iHumidity);

  #ifdef DEBUG
  Serial.print(F("Temperature = "));
  Serial.println(temperature);

  Serial.print(F("Pressure = "));
  Serial.println(pressure);
  
  Serial.print(F("Humidity = "));
  Serial.println(humidity);
  #endif
}
