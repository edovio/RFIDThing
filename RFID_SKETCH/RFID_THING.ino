/**********************************************************************************************************************
 * Sketch created by Edoardo Viola - GitHub Account: @edovio - Email: edoardo@edovio.in - http://edovio.in            *
 *            This code is under Creative Commons - Attribution 4.0 International (CC BY 4.0)                         *
 *              For any information about the project or to contribute follow it on GitHub                            *                 
 * *******************************************************************************************************************/

 /*
  RFID - NODEMCU CONFIG:
    SDA  D8
    SCK  D5
    MOSI D7
    MISO D6
    IRQ  N/D
    RST  D1  
*/

//Library for RFID
#include <SPI.h>
#include <MFRC522.h>

//Library for Mozilla IOT Gateway
#include <Arduino.h>
#include "Thing.h"
#include "WebThingAdapter.h"

//Pin Conversion for ESP8266
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15
#define D9 3
#define D10 1

//PIN CONFIG FOR RFID
#define SS_PIN D8
#define RST_PIN D1

MFRC522 mfrc522(SS_PIN, RST_PIN);

//Led Status PIN
#define ledStat D2

//TODO: Hardcode your wifi credentials here (and keep it private)
const char* ssid = "SSID";
const char* password = "PASSWORD";



WebThingAdapter adapter("w25");

const char* rFidTypes[] = {"ID_CARD", "AUTH"};
ThingDevice rfidReader("RFID_READER", "ID Tessera", "Autorizzazione");
ThingProperty idValue("ID-CARD", "", STRING);
ThingProperty authValue("AUTH", "", BOOLEAN);

bool lastOn = false;
String current;

void setup(void){
  SPI.begin(); 
  mfrc522.PCD_Init();   // Initiate MFRC522
  
  pinMode(ledStat,OUTPUT);
  Serial.begin(9600);
  Serial.println("");
  Serial.print("Connecting to \"");
  Serial.print(ssid);
  Serial.println("\"");
#if defined(ESP8266) || defined(ESP32)
  WiFi.mode(WIFI_STA);
#endif
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  bool blink = true;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledStat, blink ? LOW : HIGH); // active low led
    blink = !blink;
  }
  digitalWrite(ledStat, HIGH); // active low led

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  rfidReader.addProperty(&idValue);
  rfidReader.addProperty(&authValue);
  adapter.addDevice(&rfidReader);
  adapter.begin();
  Serial.println("HTTP server started");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.print("/things/");
  Serial.println(rfidReader.id);
}

void loop(void){
  adapter.update(); 
  ThingPropertyValue idCurrent;
  ThingPropertyValue authCurrent;

  // Look for new cards
    if (mfrc522.PICC_IsNewCardPresent()) 
    {
    // Select one of the cards
      if (mfrc522.PICC_ReadCardSerial()) 
      {
      //Show UID on serial monitor
      Serial.print("UID tag :");
      String content= "";
      byte letter;
      for (byte i = 0; i < mfrc522.uid.size; i++) 
      {
         Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
         Serial.print(mfrc522.uid.uidByte[i], HEX);
         content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
         content.concat(String(mfrc522.uid.uidByte[i], HEX));
      }
      Serial.println();
      Serial.print("Message : ");
      content.toUpperCase();
      current = (String)content.substring(1);
      idCurrent.string = &current;
      if (content.substring(1) == "49 6B 1D 28") //change here the UID of the card/cards that you want to give access
      {
            authCurrent.boolean = HIGH ;
          Serial.println("Authorized access");
          Serial.println();
        delay(2000);
      }
     
     else{
        Serial.println(" Access denied");
          authCurrent.boolean = LOW;
        delay(2000);
      }
    }
  }
  idValue.setValue(idCurrent);
  authValue.setValue(authCurrent);
} 
