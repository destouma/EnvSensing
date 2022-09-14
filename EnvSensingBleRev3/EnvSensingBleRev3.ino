#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>
#include "cactus_io_BME280_I2C.h"
// #define DEBUG 1

uint8_t pressureFormat[] = {
  0x08, // Format = UINT 32
  0x00, // Exponent = 0
  0x24, // Unit = 0x2724 = pascal
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t temperatureFormat[] = {
  0x0E, // Format = SINT16
  0x00, // Exponent = 0
  0x00, // Unit = 0x272f = celcius temperature
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

uint8_t humidityFormat[] = {
  0x06, // Format = UNINT16
  0x00, // Exponent = 0
  0xAD, // Unit = 0x27AD = percentage
  0x27, // ditto (high byte)
  0x01, // Namespace = 1 = "Bluetooth SIG Assigned Numbers"
  0x00, // Description = 0 = "unknown" (low byte)
  0x00, // ditto (high byte)
};

// Bluetooth
BLEService environmentalSensingService("181A");
BLEUnsignedLongCharacteristic pressureChar("2A6D", BLERead | BLENotify );
BLEShortCharacteristic temperatureChar("2A6E", BLERead | BLENotify );
BLEShortCharacteristic humidityChar("2A6F", BLERead | BLENotify );
BLEDescriptor pressurePresentationFormatDesc("2904",pressureFormat,7);
BLEDescriptor temperaturePresentationFormatDesc("2904",temperatureFormat,7);
BLEDescriptor humidityPresentationFormatDesc("2904",humidityFormat,7);

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
  
  pressureChar.addDescriptor(pressurePresentationFormatDesc);
  environmentalSensingService.addCharacteristic(pressureChar); 
  
  temperatureChar.addDescriptor(temperaturePresentationFormatDesc);
  environmentalSensingService.addCharacteristic(temperatureChar); 
  
  humidityChar.addDescriptor(humidityPresentationFormatDesc);
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
    delay(200);
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
