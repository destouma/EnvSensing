# Temperature, Pressure and Battery

Parts:
======
Rev 1:
- 1 Arduino MKR 1010 WIFI
- 1 Adafruit BMP 280 sensor
- 1 Bread Board
- 1 battery LI-ON 3.7V / 2000 mAH
- Some wires


Board:
======
![board](https://github.com/destouma/envsensing/blob/master/Board/EnvSensingRev1.jpg)


BLE Software:
=============
Environmental Sensing Service:
https://www.bluetooth.com/xml-viewer/?src=https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.environmental_sensing.xml

Battery Service:
https://www.bluetooth.com/xml-viewer/?src=https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Services/org.bluetooth.service.battery_service.xml


WiFi Software (http):
=====================
Http Client is posting to the rest api env-sensing-api


WiFi Software (mqtt):
=====================
[empty]


TODO:
=====
- create a new rev1 software using MQTT
