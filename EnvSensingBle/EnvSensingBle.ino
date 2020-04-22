#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

// Bluetooth
BLEService environmentalSensingService("181A");
BLEUnsignedLongCharacteristic pressureChar("2A6D", BLERead | BLENotify);
BLEShortCharacteristic temperatureChar("2A6E", BLERead | BLENotify);

BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",BLERead | BLENotify);


// BMP 280 sensor
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)

Adafruit_BMP280 bmp;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);    // initialize serial communication
  while (!Serial);

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */


  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  BLE.setDeviceName("ES");
  BLE.setLocalName("EnvironmentalSensing");
  
  BLE.setAdvertisedService(environmentalSensingService); 
  environmentalSensingService.addCharacteristic(pressureChar); 
  environmentalSensingService.addCharacteristic(temperatureChar); 
  BLE.addService(environmentalSensingService); 
  
  batteryService.addCharacteristic(batteryLevelChar);
  BLE.addService(batteryService);
  
  pressureChar.writeValue(0);
  temperatureChar.writeValue(0);
  batteryLevelChar.writeValue(0);
  
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    Serial.println();

    updateCharValues();
    updateBatteryLevel();
    Serial.println();

  }
}

void updateCharValues(){
  unsigned long pressure = bmp.readPressure(); 
  short temperature = bmp.readTemperature() * 100;
  
  pressureChar.writeValue(pressure);
  temperatureChar.writeValue(temperature);
  
  Serial.print(F("Temperature = "));
  Serial.println(temperature);

  Serial.print(F("Pressure = "));
  Serial.println(pressure);
}

void updateBatteryLevel() {
  int battery = analogRead(ADC_BATTERY);
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  
  batteryLevelChar.writeValue(batteryLevel);
  Serial.print("Battery Level % is now: "); // print it
  Serial.println(batteryLevel);
}
