#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Sensor
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

int battery=0;
int batteryLevel=0;
short temperature=0;
unsigned long pressure=0;
unsigned long counter=0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
  
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     
                  Adafruit_BMP280::SAMPLING_X2,  
                  Adafruit_BMP280::SAMPLING_X16,  
                  Adafruit_BMP280::FILTER_X16,     
                  Adafruit_BMP280::STANDBY_MS_500); 


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
  delay(1000);
  display.clearDisplay();  
  display.setTextSize(4); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.println(bmp.readTemperature());
  display.display();

  delay(1000);
  display.clearDisplay();  
  display.display();

  delay(1000);
  display.clearDisplay();  
  display.setTextSize(4); 
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  pressure = bmp.readPressure()/100;
  display.println(pressure);
  display.display();

  delay(1000);
  display.clearDisplay();  
  display.display();
  
}
