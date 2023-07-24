
#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>   
#include <PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire DS1820(D5);
 
void showAddress(byte address[8])
  {
  // Im Array address enthaltene Daten kompakt sedezimal ausgeben
  byte i;

  if (OneWire::crc8(address, 7) != address[7])
    { Serial.println("hat keinen gueltigen CRC!"); }
  else
    {
    //alles ist ok, anzeigen
    for (i = 0; i < 8; i++)
      {
      if (address[i] <= 0xF)
        { Serial.print("0"); }
      Serial.print(address[i],HEX);
      }
    Serial.println("");
    }
  }

void lookUpSensors()
  {
  // Alle DS18x20-Sensoren suchen, Adresse ausgeben
  byte address[8];
  bool found = false;
  bool detect = false;
 
  Serial.println("--Suche gestartet--");
  while (DS1820.search(address))
    {
    detect = false;
    switch (address[0])
      {
      case 0x10:
        Serial.print("DS18S20: ");  // oder alter DS1820
        detect = true;
        break;
      case 0x28:
        Serial.print("DS18B20: ");
        detect = true;
        break;
      case 0x22:
        Serial.print("DS1822: ");
        detect = true;
        break;
      default:
        Serial.println("Device ist kein DS18x20-Sensor.");
      } 
    // Adresse anzeigen, wenn ein Temperatursensor gefunden wurde
    if (detect)
      {
      found = true;
      showAddress(address);
      } // ... if detect
    }// ... while
  if (!found)
    { Serial.println("Keine Sensoren gefunden"); }
  Serial.println("--Suche beendet--");
  }


void setup(void)
  {
  Serial.begin(115200);
  lookUpSensors();
  }

void loop(void)
  {
  // nichts tun ...
  }

  