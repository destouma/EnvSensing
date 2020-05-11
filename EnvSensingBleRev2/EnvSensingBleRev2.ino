#include <ArduinoBLE.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


const int ledPin = LED_BUILTIN; // set ledPin to on-board LED

// Bluetooth
BLEService environmentalSensingService("181A");
BLEUnsignedLongCharacteristic pressureChar("2A6D", BLERead | BLENotify | BLEWrite);
BLEShortCharacteristic temperatureChar("2A6E", BLERead | BLENotify | BLEWrite);
BLEDescriptor configDesc("2902", "config");

BLEService batteryService("180F");
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",BLERead | BLENotify);

BLEService ledService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service
BLEByteCharacteristic ledCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// BMP 280 sensor
#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)
Adafruit_BMP280 bmp;

// OLED screen
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

  ledService.addCharacteristic(ledCharacteristic);
  BLE.addService(ledService);

  pressureChar.writeValue(0);
  temperatureChar.writeValue(0);
  batteryLevelChar.writeValue(0);
  
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  display.display();
  delay(2000);

  // Clear the buffer
  display.clearDisplay();
  display.display();
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {

    
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    Serial.println();

    if (ledCharacteristic.written()) {
        // update LED, either central has written to characteristic or button state has changed
        if (ledCharacteristic.value()) {
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);
        } else {
          Serial.println("LED off");
          digitalWrite(ledPin, LOW);
        }
      }

    updateCharValues();
    updateBatteryLevel();
    Serial.println();
    
    display.clearDisplay();  
    display.setTextSize(4); 
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(5, 10);
    display.println("BLE");
    display.display();
  }
  else
  {
    display.clearDisplay();
    display.display();
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
