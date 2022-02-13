
#include "cactus_io_BME280_I2C.h"

// BME 280
BME280_I2C bme(0x76); // I2C using address 0x76


void setup() {
  //SERIAL init
  Serial.begin(9600);
  while (!Serial) {
    delay(1000); 
  }

  // BME280 sensor
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }
  bme.setTempCal(-1);
}

void loop() {
  bme.readSensor();
  Serial.print(bme.getPressure_MB()); Serial.print(" mb\t"); // Pressure in millibars
  Serial.print(bme.getHumidity()); Serial.print(" %\t\t");
  Serial.print(bme.getTemperature_C()); Serial.print(" *C\t");
  Serial.print(bme.getTemperature_F()); Serial.println(" *F\t");
 
}
