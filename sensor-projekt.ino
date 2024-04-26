/*
  Name:     Magnus Drolshagen
  Klasse:   E2FS1
  Datum:    22.04.2024
  Version:  1.0.1
*/

#include <Wire.h>  // Wire Bibliothek für I2C-Display
#include <WiFi.h>    //Einbinden der WiFi-Bibliothek
#include <BH1750.h>  //Einbinder der Biliothek für Lichtsensor
#include <DHT.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

AsyncWebServer server(80);      //Starten des Servers

char* ssid = "IoT_Elektro";        //Anmeldedaten fuer die WLAN Verbindung
char* passwort = "DIoT-UadKASmS";

//--- Disstanz Anfang ---
const int distTrigPin = 5;      //Sensorvariablen fuer Entfernungsmessung festlegen
const int distEchoPin = 18;     
#define SOUND_SPEED 0.034
long distDuration;
float distDistanceCm;
//--- Distanz Ende ---


//--- LUX Anfang ---
BH1750 lux;                     //Variable fue rHelligkeit festlegen
//--- LUX Ende ---

//--- Luft Anfang ---
DHT dht(15, DHT11);             //Variable fuer Tep und Feuchte festlegen
//--- Luft Ende ---

int val_digital;                //Variablen fuer Messwerte festlegen
int val_analog;
float val_lux;
float temp;
float humidity;
float hic;



String iconhell = "hidden";   //Variablen fuer Warnsymbole festlegen
String icontemp = "hidden";
String iconhumid = "hidden";
String icondist = "hidden";
String iconsound = "hidden";


void setup() {
  Serial.begin(115200);       //Starten des Seriellen Monitor
  Wire.begin();

  Serial.print("Verbindungsaufbau zu ");
  Serial.println(ssid);

  WiFi.begin(ssid, passwort);       //WLAN-Verbindung starten
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(WiFi.status());
    delay(500);
    Serial.print(".");
  }

  Serial.print("Verbindung hergestellt.\n Lokale IP: ");
  Serial.println(WiFi.localIP());                           //IP und Netzwerkdaten ausgeben
  Serial.print("ESP Board MAC Adresse: ");
  Serial.println(WiFi.macAddress());
  Serial.println(WiFi.subnetMask());

  if (!SPIFFS.begin()) {                                            //Dateisystem auf dem ESP32 starten
    Serial.println("An Error has occurred while mounting SPIFFS!");
  }

  //Die Antworten des Servers definieren
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {                  //Webdateien an den ESP32 senden
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css");
  });
  server.on("/warning.png", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/warning.png");
  });
  server.onNotFound([](AsyncWebServerRequest * request) {
    request->send(404, "text/plain", "Seite nicht vorhanden!");
  });
  server.begin();  //Den Server starten mit der Methode begin()

  
  pinMode(distTrigPin, OUTPUT);       //Sensorenerfassung starten
  pinMode(distEchoPin, INPUT);
  lux.begin();
  dht.begin();

}

void loop() {
  werte_auslesen();
  werte_anzeigen();

  if (val_lux < 500){           //Die erfassten Werte interpretieren
    iconhell = "visible";
  } else if (val_lux > 750){
    iconhell = "visible";
  } else {
    iconhell = "hidden";
  }

  if (temp < 22){
    icontemp = "visible";
  } else if (temp > 24){
    icontemp = "visible";
  } else {
    icontemp = "hidden";
  }

  if (humidity < 40){
    iconhumid = "visible";
  } else if (humidity > 60){
    iconhumid = "visible";
  } else {
    iconhumid = "hidden";
  }

    if (distDistanceCm < 45){
    icondist = "visible";
  } else if (distDistanceCm > 80){
    icondist = "visible";
  } else {
    icondist = "hidden";
  }
}




void distanz_auslesen() {             //Distanz erfassen
  digitalWrite(distTrigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(distTrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distTrigPin, LOW);
  distDuration = pulseIn(distEchoPin, HIGH);          //Zeitspanne messen
  distDistanceCm = distDuration * SOUND_SPEED / 2;      //Berechnung
}

void lux_auslesen() {
  val_lux = lux.readLightLevel();             //Helligkeit erfassen
}

void luft_auslesen() {
  temp = dht.readTemperature();                 //Luftwerte erfassen
  humidity = dht.readHumidity();
  hic = dht.computeHeatIndex(temp, humidity, false);      //gefuehlte Temperatur erfassen
}


void werte_auslesen() {
  distanz_auslesen();
  lux_auslesen();
  luft_auslesen();

  delay(1000);
}

void werte_anzeigen() {       //Darstellung der Werte auf den Seriellen Monitor
  Serial.print("Ger. ");
  Serial.print(val_analog);
  Serial.print(" / ");
  Serial.print(val_digital);
  Serial.print("  |  ");
  Serial.print(distDistanceCm);
  Serial.print(" cm  |  ");
  Serial.print(val_lux);
  Serial.print(" lx  |  ");
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.print("°C  |  ");
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %  |  ");
  Serial.print("HeatIndex: ");
  Serial.print(hic);
  Serial.println(F(" °C"));
}


String processor(String var) {
  if (var == "TEMP") {
    return String(temp);
  }
  if (var == "LUX") {
    return String(val_lux);
  }
  if (var == "ICONHELL") {
    return String(iconhell);
  }
  if (var == "ICONTEMP") {
    return String(icontemp);
  }
  if (var == "HUMIDITY") {
    return String(humidity);
  }
  if (var == "ICONHUMID") {
    return String(iconhumid);
  }
    if (var == "DISTANCE") {
    return String(distDistanceCm);
  }
  if (var == "ICONDIST") {
    return String(icondist);
  }
  if (var == "HINDEX") {
    return String(hic);
  }
}
