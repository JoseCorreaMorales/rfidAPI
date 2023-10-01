#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <PN532_I2C.h>
#include <PN532.h>

#include <NfcAdapter.h>

//Conexion wifi

#define SSID "f1f7ec"
#define PSK "272939908"
#define URL "http://192.168.0.47/rfidAPI/"
//#define SSID "IOT"
//#define PSK "CNtec2023"
//#define URL "http://172.18.7.226/rfidAPI/"
//Usuario de login
#define USER "admin"
#define PASS "123"

//Alto y ancho de pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

//pines de conexion I2C en el ESP32 (No deben cabiarese)
#define I2C_SDA 21
#define I2C_SCL 22
//Led integrado(azul) del ESP32
#define LED_ST 2
#define LED_RED 12
#define LED_GREEN 13
#define BUZZER 14

//variable ID unico de la tarjeta RF MiFare
//la variable de llama tagId32 y como es de tipo union, se puede usar tagId32.array para obtener sus bytes o
//tagId32.integer para obtener su valor entero

union ArrayToInteger {
  byte array[4];
  uint32_t integer;
} tagId32;

//Permite conectar al mismo cable dos dispositivos
TwoWire WireI2C = TwoWire(0);

//permite controlar la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &WireI2C, -1);

//Permite controlar el lector NFC/RFID
PN532_I2C pn532_i2c(WireI2C);
PN532 nfc(pn532_i2c);

//buffer temporal para leer el ID de las tarjetas
boolean success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidLength;

WiFiMulti wifiMulti;
//variables de sesion
String jwt;
bool sesion;

void setup(void) {
  //Inicializa serial y manda imprimir un mensaje
  Serial.begin(115200);
  Serial.println("Setup....");

  //inicializa led integrado y lo apaga
  pinMode(LED_ST, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_ST, LOW);
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  //inicializa cables I2C de pantalla y NFC/RFID
  WireI2C.begin(I2C_SDA, I2C_SCL, 10000);

  //inicializa pantalla
  display.setRotation(2);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 fallo conexión"));
    for (;;);
  }
  delay(1000);

  //imprime mensaje en pantalla
  printLCD("Conectando a red....");

  //conecta a wifi
  wifiMulti.addAP(SSID, PSK);
  sesion = false;
  while (wifiMulti.run() != WL_CONNECTED && millis() < 10000);

  //imprime mensaje en pantala de iniciando e inicializa lector NFC/RFID
  printLCD("Iniciando....");
  nfc.begin();

  //Comprueba si se inicializo correctamente el lector NFC/RFID
  uint32_t versiondata = nfc.getFirmwareVersion();

  if (!versiondata) {
    Serial.println("Didn't find PN53x board");
    printLCD("ERROR RF");
    delay(1000);
    while (1);
  }
  //Se configura el lector NFC/RFID para tarjetas MiFare clasicas
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
  //Enciende led integrado e imprime mensaje de BIENVENIDO en pantalla y en puerto serial
  digitalWrite(LED_ST, HIGH);
  printLCD("BIENVENIDO");
  Serial.println("Sistema Inicializado");
}

void loop() {
  readNFC();
}

void readNFC() {

  //Lee si hay tarjeta RFID, en tal caso devuelve un true, si no hay tarjeta es un false
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 50);
  Serial.println(success);
  printLCD("BIENVENIDO");

  if (success) {
    Serial.println(uidLength);
    //copia el arreglo de bytes de la tarjeta leida a la variable tagId32
    //esto permite que se convirta automaticamente el arreglo de bytes a un valor entero
    tagId32.array[0] = uid[0];
    tagId32.array[1] = uid[1];
    tagId32.array[2] = uid[2];
    tagId32.array[3] = uid[3];

    //imprime el ID de la tarjeta en serial y en pantalla OLED
    Serial.println(tagId32.integer);
    printLCD("COBRANDO...");
    printLCD(String(tagId32.integer), 2);
    song();
    //Si esta conectado a WiFi ejecuta login para iniciar sesion
    //y pay para madar el ID de la tarjeta al servidor
    //sino imprime en pantalla un mensaje de sin red
    if ((wifiMulti.run() == WL_CONNECTED)) {
      if (!sesion) login();
      pay();
    } else {
      digitalWrite(LED_RED, HIGH);
      delay(5000);
      printLCD("SIN RED...");
      digitalWrite(LED_RED, LOW);
    }
  }
  delay(500);
}


void tone(byte pin, int freq) {
  ledcSetup(1, 2000, 8);
  ledcAttachPin(pin, 1);
  ledcWriteTone(1, freq);
}
void noTone(byte pin) {
  tone(pin, 0);
}

//Funcion para imprimir mensajes en pantalla
void printLCD(String x) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();
  printLCD(x, 0);
}

//funcion para imprimier mensajes en una segunda linea en pantalla
void printLCD(String x, int linea) {
  display.setCursor(0, linea * 16 + 2);
  display.println(x);
  display.display();
}

void song() {
  tone(BUZZER, 1000);  // Tono alto
  delay(200);
  noTone(BUZZER);
  delay(100);
  tone(BUZZER, 1500);  // Tono aún más alto
  delay(200);
  noTone(BUZZER);
}

//funcion para hacer login y obtener el token JWT del sevidor
void login() {
  HTTPClient http;
  //url de la api o servicio rest del login
  //en arduino las constantes se concatenan sin ningun operador
  //por ejemplo aqui se hace un get a PHP http:  //172.16.253.30/smartpay/login.php?user=admin&amp;pass=123&quot;
  http.begin(URL "login.php?user=" USER "&pass=" PASS);
  int httpCode = http.GET();

  Serial.println("Haciendo login");
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      //payload contiene lo que el server haya enviado, generalmente un json
      String payload = http.getString();
      Serial.println(payload);
      //se extrae del json el token JWT mediante la funcion buscarJson()
      jwt = buscarJson("jwt", payload);
      Serial.println(jwt);
      sesion = true;
    } else if (httpCode == 401) {
      //si el servidor contesta un codigo HTTP 401 significa que no tiene autorizacion o permiso de acceder a server
      sesion = false;
    }
  }
  http.end();
}
//funcion para enviar el ID de la tarjeta al server
void pay() {
  HTTPClient http;
  //url a donde se quiera mandar el id
  http.begin(URL "pay.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", "Bearer " + jwt);
  //envia un json con el id de la tarjeta similar a {&quot;tagid&quot;:&quot;0000000000&quot;}
  int httpCode = http.POST("tagid=" + String(tagId32.integer));

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
      Serial.println("Registro exitoso");

      String msg = buscarJson("costo", payload);
      Serial.println(msg);
      printLCD("PAGO:" + msg);
      msg = buscarJson("saldo", payload);
      Serial.println(msg);
      printLCD("SALDO:", 1);
      printLCD(msg, 2);
      digitalWrite(LED_GREEN, HIGH);
      delay(5000);
      printLCD("BIENVENIDO");
      digitalWrite(LED_GREEN, LOW);
  

      return;

    } else {
      Serial.println("No se pudo cobrar" + httpCode);
    }
  } else {
    Serial.println("No hay conexion con servidor " + httpCode);
    
  }
  printLCD("ERROR AL COBRAR");
  digitalWrite(LED_RED, HIGH);
  delay(5000);
  printLCD("BIENVENIDO");
  digitalWrite(LED_RED, LOW);
}
//permite buscar el valor de una clave en un json, por ejemplo
//en el json = {&quot;control&quot;: &quot;S22030001&quot;, &quot;nombre&quot;: &quot;JUAN&quot;}
//buscarJson(&quot;control&quot;, json) devoveria &quot;S22030001&quot;

String buscarJson(String clave, String json) {
  int i, f;
  String valor;
  i = json.indexOf(clave);
  i = json.indexOf(":", i);
  i = json.indexOf("\"", i) + 1;
  f = json.indexOf("\"", i);
  return json.substring(i, f);
}