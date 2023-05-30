
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

/* RFID libary */
#include <MFRC522.h>

#define SSID "IOT"
#define PSK "CNtec2023"
#define URL "http://172.18.0.2/"


LiquidCrystal_I2C lcd(0x3f, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class MFRC522 - reader
WiFiMulti wifi;
 

String jwt;
bool sesion;

double saldoGlobal;



/* -------------------------------  SETUP-------------------------- */
void setup() {
  Serial.begin(9600);
  wifi.addAP( SSID, PSK);
  Serial.print("Conectando al WiFi...");
  //wifiMulti.addAP(ssid, password); to add multiple AP Connections

  while(wifi.run() != WL_CONNECTED);
  Serial.println("Conectado")


  SPI.begin();        // start the communication to the reader
  rfid.PCD_Init();    // start the RFID reader
  
}





void loop() {

  if(sesion) registro();
  else login();
  delay(10000);

/* ----------------------------- CKECK --------------------------------------*/
 if (WiFi.status() != WL_CONNECTED) {
    // Si no hay WiFi, no hacemos nada
    Serial.println("No hay WiFi");
    return;
  } 

  /* ------------------------- RFID LOGIC-------------------------------- */
  // Verifica si se detecta una tarjeta RFID presente y leemos su UID
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    // read UID of RFID card
    Serial.print("UID de la tarjeta: ");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "); //formatting the output
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();

    rfid.PICC_HaltA(); //stop reading
  }
}

void login() {
  HTTPClient http;
  http.begin(URL "login"); //HTTP
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST("{\"username\":\"\admin\", \"password\":\"123\"}");

        if(httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            if(httpCode == 200) {
                String payload = http.getString();  
                // Serial.println(payload);
              int p;
              p = payload.indexOf("}");
              jwt = payload.substring(10, p-2);
              Serial.println(jwt);
              sesion = true;
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
          http.end();
}


void registro() {
  HTTPClient http;
  http.begin(URL "sensores"); //HTTP
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", jwt);
  int httpCode = http.POST("{\"name\":\"\ULTRA\", \"value\":\"" + String(distancia) + "\"}");

        if(httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            if(httpCode == 201) {
                String payload = http.getString();  //el objeto completo viene el el payload
                Serial.println(payload);
              
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
          http.end();
}


void guardarSaldo() {

  HTTPClient http;
  http.begin(URL "saldo"); //HTTP
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", jwt);
  int httpCode = http.POST("{\"saldo\":\"" + String(saldoGlobal) + "\"}");

        if(httpCode > 0) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            if(httpCode == 201) {
                String payload = http.getString();  //el objeto completo viene el el payload
                Serial.println(payload);
              
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }
          http.end();
}














