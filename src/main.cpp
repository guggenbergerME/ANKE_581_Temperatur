#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>   
#include <PCF8574.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void wifi_setup                 ();
void callback                   (char* topic, byte* payload, unsigned int length);
void reconnect                  ();
void temp_messen                ();
void mqbstt_callback_aufrufen   ();

//////////////////////////////////////////////////////////////////////////// PCF8574 Adresse
PCF8574 pcf8574(0x20);
// Wemso SDA und SDC Pins

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_temp_messen = 0; // Spannung Messen
unsigned long interval_temp_messen = 15000; 

unsigned long previousMillis_mqtt_callback = 0; // Spannung Messen
unsigned long interval_mqtt_callback = 100; 

/////////////////////////////////////////////////////////////////////////// mqtt variable
char msgToPublish[60];
char stgFromFloat[10];
char textTOtopic[60];
const char* kartenID = "ANKE581_Temp_001";

/////////////////////////////////////////////////////////////////////////// WIRE Bus
#define ONE_WIRE_BUS D5
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

/////////////////////////////////////////////////////////////////////////// Temp. Sensor HEX zuweisen
DeviceAddress temp_abgas         = { 0x28, 0xDB, 0x51, 0x80, 0x1E, 0x13, 0x01, 0xFA }; 
//                                  28 DB 51 80 1E 13 01 FA
const char* topic_temp_abgas     = "Heizung/Holzvergaser/temp/abgas";

DeviceAddress temp_kessel         = { 0x28, 0xB0, 0xF7, 0x93, 0x1E, 0x13, 0x01, 0xDE }; 
//                                  28 B0 F7 93 1E 13 01 DE
const char* topic_temp_kessel     = "Heizung/Holzvergaser/temp/kessel";

DeviceAddress temp_rl            = { 0x28, 0xF3, 0xF5, 0x7C, 0x1B, 0x13, 0x01, 0x97 }; 
//                                  28 F3 F5 7C 1B 13 01 97
const char* topic_temp_rl        = "Heizung/Holzvergaser/temp/ruecklauf";

DeviceAddress temp_vl            = { 0x28, 0xFF, 0xFE, 0x3E, 0xA1, 0x16, 0x04, 0x6B }; 
//                                  28 FF FE 3E A1 16 04 6B
const char* topic_temp_vl        = "Heizung/Holzvergaser/temp/vorlauf";

WiFiClient espClient;
PubSubClient client(espClient);

/////////////////////////////////////////////////////////////////////////// wifi daten
const char* ssid = "ANKE581";
const char* password = "q8d67nH5mH";
const char* mqtt_server = "192.168.1.3";

/////////////////////////////////////////////////////////////////////////// Callback
void callback(char* topic, byte* payload, unsigned int length) {
             

}

/////////////////////////////////////////////////////////////////////////// Reconnect
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Baue Verbindung zum mqtt Server au. IP: ");
    Serial.print(mqtt_server);
    Serial.println("");
    // Create a random client ID
    String clientId = "HolzvergaserSteuerung-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("MQTT Verbunden ....");
      // ... and resubscribe
      client.subscribe("Heizung/Holzvergaser/geblaese/IN");
      client.subscribe("Heizung/Holzvergaser/pumpe/heizkreis/IN");
      client.subscribe("Heizung/Puffer/pumpe/laden/IN");
      client.subscribe("Heizung/Puffer/pumpe/entladen/IN");    
      client.subscribe("Licht/Keller/Heizungsraum/IN"); 
      client.subscribe("Heizung/Holzofen/Wassertasche/pumpe/IN"); 
      client.subscribe("Heizung/OG/pumpe/IN"); 
      client.subscribe("Heizung/Fussboden/pumpe/IN"); 

         
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/////////////////////////////////////////////////////////////////////////// SETUP - Wifi
void wifi_setup() {
// Verbindung zum WiFI aufbauen

  Serial.print("Verbindung zu SSID -> ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Erfolgreich mit dem WiFi verbunden!");
  Serial.print("DHCP Adresse der Relaikarte : ");
  Serial.println(WiFi.localIP());
  Serial.print("ID der Relaiskarte: ");
  Serial.println(kartenID);
}

/////////////////////////////////////////////////////////////////////////// SETUP
void setup() {

//*********************************************** MQTT Broker
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

//*********************************************** Serielle Kommunikation starten
  Serial.begin(115200);

  //*************************************************************** Temp Sensor auslesen
sensors.begin();
sensors.setResolution(temp_abgas, 9);
sensors.setResolution(temp_kessel, 9);
sensors.setResolution(temp_rl, 9);
sensors.setResolution(temp_vl, 9);


//*********************************************** Wifi Setup
wifi_setup();

//*********************************************** pcf8574
  pcf8574.pinMode(P0, OUTPUT);
  pcf8574.pinMode(P1, OUTPUT);
  pcf8574.pinMode(P2, OUTPUT);
  pcf8574.pinMode(P3, OUTPUT); 
  pcf8574.pinMode(P4, OUTPUT); 
  pcf8574.pinMode(P5, OUTPUT); 
  pcf8574.pinMode(P6, OUTPUT); 
  pcf8574.pinMode(P7, OUTPUT);     
  pcf8574.begin();

  //*********************************************** Relais OFF
pcf8574.digitalWrite(P0, !LOW);
pcf8574.digitalWrite(P1, !LOW);
pcf8574.digitalWrite(P2, !LOW);
pcf8574.digitalWrite(P3, !LOW);
pcf8574.digitalWrite(P4, !LOW);
pcf8574.digitalWrite(P5, !LOW);
pcf8574.digitalWrite(P6, !LOW);
pcf8574.digitalWrite(P7, !LOW);

}

/////////////////////////////////////////////////////////////////////////// Temperatur auslesen
void temp_messen() {

sensors.requestTemperatures();

////////////////////////////////////////////////////////// Sensor 1
  int currentTemp1 = sensors.getTempC(temp_abgas);
  dtostrf(currentTemp1, 4, 2, stgFromFloat);
 Serial.println(currentTemp1);
   if ((currentTemp1 == -127)||(currentTemp1 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_temp_abgas);
  client.publish(textTOtopic, msgToPublish);
 }

////////////////////////////////////////////////////////// Sensor 2
  int currentTemp2 = sensors.getTempC(temp_kessel);
  dtostrf(currentTemp2, 4, 2, stgFromFloat);
 Serial.println(currentTemp2);
   if ((currentTemp2 == -127)||(currentTemp2 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_temp_kessel);
  client.publish(textTOtopic, msgToPublish);
 }


////////////////////////////////////////////////////////// Sensor 3
  int currentTemp3 = sensors.getTempC(temp_rl);
  dtostrf(currentTemp3, 4, 2, stgFromFloat);
 Serial.println(currentTemp3);
   if ((currentTemp3 == -127)||(currentTemp3 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_temp_rl);
  client.publish(textTOtopic, msgToPublish);
 }

 ////////////////////////////////////////////////////////// Sensor 4
  int currentTemp4 = sensors.getTempC(temp_vl);
  dtostrf(currentTemp4, 4, 2, stgFromFloat);
 Serial.println(currentTemp4);
   if ((currentTemp4 == -127)||(currentTemp4 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_temp_vl);
  client.publish(textTOtopic, msgToPublish);
 }


}

/////////////////////////////////////////////////////////////////////////// mqtt Callback aufrufen
void mqtt_callback_aufrufen() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

/////////////////////////////////////////////////////////////////////////// LOOP
void loop() {

   //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ mqtt Callback aufrufen
  if (millis() - previousMillis_mqtt_callback> interval_mqtt_callback) {
      previousMillis_mqtt_callback= millis(); 
      // Prüfen der Panelenspannung
      Serial.println("mqtt Callback aufrufen");
      mqtt_callback_aufrufen();
    } 


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Temperatur messen
  if (millis() - previousMillis_temp_messen > interval_temp_messen) {
      previousMillis_temp_messen= millis(); 
      // Prüfen der Panelenspannung
      Serial.println("Temperatur messen");
      temp_messen();
    }

 
}
