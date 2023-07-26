#include <Arduino.h>
#include <SPI.h>
#include <ESP8266WiFi.h> 
#include <PubSubClient.h>   
#include <OneWire.h>
#include <DallasTemperature.h>

/////////////////////////////////////////////////////////////////////////// Funktionsprototypen
void loop                       ();
void wifi_setup                 ();
void callback                   (char* topic, byte* payload, unsigned int length);
void reconnect                  ();
void temp_messen                ();
void mqbstt_callback_aufrufen   ();

/////////////////////////////////////////////////////////////////////////// Schleifen verwalten
unsigned long previousMillis_temp_messen = 0; // Spannung Messen
unsigned long interval_temp_messen = 2500; 

unsigned long previousMillis_mqtt_callback = 0; // Spannung Messen
unsigned long interval_mqtt_callback = 500; 

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
DeviceAddress temp_sensor_1         = { 0x28, 0xF3, 0xF5, 0x7C, 0x1B, 0x13, 0x01, 0x97 }; 
//                                  28 DB 51 80 1E 13 01 FA
const char* topic_sensor_1     = "ANKE581/temp001/Sensor1";

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
    //Serial.print("Baue Verbindung zum mqtt Server auf. IP: ");
    // Attempt to connect
    if (client.connect(kartenID,"anke581","Sa,7+CGPfA9e6)y!")) {
      //Serial.println("connected");
      ////////////////////////////////////////////////////////////////////////// SUBSCRIBE Eintraege
                                            
        client.subscribe(topic_sensor_1);
        client.publish(topic_sensor_1, "Online"); // mqtt Bekanntgabe 
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(2000);
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
sensors.setResolution(temp_sensor_1, 9);

//*********************************************** Wifi Setup
wifi_setup();

}

/////////////////////////////////////////////////////////////////////////// Temperatur auslesen
void temp_messen() {

sensors.requestTemperatures();

////////////////////////////////////////////////////////// Sensor 1
  int currentTemp1 = sensors.getTempC(temp_sensor_1);
  dtostrf(currentTemp1, 4, 2, stgFromFloat);
 Serial.println(currentTemp1);
   if ((currentTemp1 == -127)||(currentTemp1 == 85))  { 
     } 
    else 
        {   
  sprintf(msgToPublish, "%s", stgFromFloat);
  sprintf(textTOtopic, "%s", topic_sensor_1);
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
      //Serial.println("mqtt Callback aufrufen");
      mqtt_callback_aufrufen();
    } 


  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Temperatur messen
  if (millis() - previousMillis_temp_messen > interval_temp_messen) {
      previousMillis_temp_messen= millis(); 
      // Prüfen der Panelenspannung
      //Serial.println("Temperatur messen");
      temp_messen();
    }

 
}
