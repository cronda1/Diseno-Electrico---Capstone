#include <Wire.h>
#include "funciones.h"
#include "MAX30100_PulseOximeter.h"
#include "SparkFunCCS811.h"
#include <OneWire.h> 
#include <DallasTemperature.h>
#include "LiquidCrystal_I2C.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "time.h"
#include <Vector.h>
#include <HTTPUpdate.h>

#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define STATE_MEDICION 1
#define STATE_FOTO 2
#define STATE_AUDIO 3
#define STATE_SEND 4
#define STATE_MENU 5
#define STATE_OTA 6

#define boton_alimento  32
#define boton_farmaco  33
#define boton_evacuar  14
#define boton_orinar  12

#define CCS811_ADDR 0x5A            // I2C Address CCS811.
#define LCD_ADDR 0x27               // I2C Address LCD.
#define PIN_TEMPERATURA 2
#define REPORTING_PERIOD_MS 1000 

using namespace std;
int valor;
vector<int> v1;
vector<int>::iterator it;
vector<int> v2;
vector<int>::iterator it2;
vector<int> v3;
vector<int>::iterator it3;
int cont = 0;
bool lista_llena = false;
float suma = 0;
float promedio = 0;
float frecuencia_resp = 0;
int var1 = 0;
int var2 = 0;
int tiempo_resp = 0;

int t1 = 0;
int t2 = 0;
int tiempo = 0;

int totalLength;       //total size of firmware
int currentLength = 0; //current size of written firmware


//INSTANCIAMOS COMUNICACIÓN ONEWIRE PARA SENSOR DE TEMPERATURA
DeviceAddress DS18B20_ADDR = {0x28, 0x25, 0x93, 0x1, 0x0, 0x0, 0x0, 0x35};
const int oneWireBus = PIN_TEMPERATURA;                                     
OneWire oneWire(oneWireBus);                                                

//INSTANCIAMOS LAS CLASES DE LOS SENSORES
PulseOximeter pox;
CCS811 mySensor(CCS811_ADDR);
DallasTemperature sensors (&oneWire);
LiquidCrystal_I2C lcd(LCD_ADDR,20,4);

WiFiClient client;
HTTPClient http;


//VARIABLES PARA SENSORES
int error_max = 0;
int error_ccs = 0;
int error_ds1 = 0;
float HR;
int spo2;
int co2;
float temp;
uint8_t estado = 1;
uint32_t tiempo1 = 0;
uint32_t tiempo2 = 0;
uint32_t tf = 0;
uint32_t tsLastReport = 0;

//VARIABLES LÓGICA BOTONES
volatile bool alimento;
volatile bool farmaco;
volatile bool evacuar;
volatile bool orinar;
volatile bool alimento2;
volatile bool farmaco2;
volatile bool evacuar2;
volatile bool orinar2;
String hora_alimento = "0";
String hora_farmaco = "0";
String hora_evacuar = "0";
String hora_orinar = "0";

//VARIABLES COMUNICACION SERVIDOR 
const char* ssid = "ronda";    //"Loreto";    //AndroidAP5337   //"Trini's Galaxy Z Flip"
const char* password = "rondance";           //"loreto33";  //villavilla
const char* serverName = "http://69.164.216.230:3000/sensores";  //"http://192.168.1.106:1880/update-sensor";
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

//SEVIDOR DE DONDE SE OBTIENE LA HORA 
const char* ntpServer = "pool.ntp.org";
unsigned long epochTime; 
const long gmtOffset_sec = -18000;
const int daylightOffset_sec = 3600;

///////////////////////////////////////// INTERRUPCIONES //////////////////////////////////////////////

//Interrupcion Boton Alimento
void IRAM_ATTR isr1() {
  alimento = true;
  alimento2 = true;
  detachInterrupt(digitalPinToInterrupt(boton_alimento));
}
//Interrupcion Boton Farmaco
void IRAM_ATTR isr2() {
  farmaco = true;
  farmaco2 = true;
  detachInterrupt(digitalPinToInterrupt(boton_farmaco));
}
//Interrupcion Boton Evacuar
void IRAM_ATTR isr3() {
  evacuar = true;
  evacuar2 = true;
  detachInterrupt(digitalPinToInterrupt(boton_evacuar));
}
//Interrupcion Boton Orinar
void IRAM_ATTR isr4() {
  orinar = true;
  orinar2 = true;
  detachInterrupt(digitalPinToInterrupt(boton_orinar));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//FUNCION PARA OBTENER LA HORA
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void updateFirmware(uint8_t *data, size_t len){
  Update.write(data, len);
  currentLength += len;
  // Print dots while waiting for update to finish
  Serial.print('.');
  // if current length of written firmware is not equal to total firmware size, repeat
  if(currentLength != totalLength) return;
  Update.end(true);
  Serial.printf("\nUpdate Success, Total Size: %u\nRebooting...\n", currentLength);
  // Restart ESP32 to see changes 
  ESP.restart();
}

String printLocalTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return  "00";
  }
  String tiempo;
  char timeHour[3];
  char f = ':';
  strftime(timeHour,3, "%H", &timeinfo);
  char timeMinute[3];
  strftime(timeMinute,3, "%M", &timeinfo);
  tiempo = String(timeHour) + String(timeMinute);
  return tiempo;
}


/////////////////////////////////////// LOOP SEGUNDO NUCLEO ESP32 ////////////////////////////////////

void miTarea2 (void *pvParameters){
  for(;;){
    if ((millis() - lastTime) > timerDelay) {
      if(WiFi.status()== WL_CONNECTED) {
        tiempo1 = millis();

        if (alimento == true) {
          //lcd.clear();
          //lcd.setCursor(0, 0);
          //lcd.print("ALIMENTO:");
          hora_alimento = printLocalTime();
          Serial.println(hora_alimento);
          attachInterrupt(digitalPinToInterrupt(boton_alimento), isr1, RISING);
          alimento = false;
        }
        if (farmaco == true) {
          //lcd.clear();
          //lcd.setCursor(0, 0);
          //lcd.print("FARMACO");
          hora_farmaco = printLocalTime();
          Serial.println(hora_farmaco);
          attachInterrupt(digitalPinToInterrupt(boton_farmaco), isr2, RISING);
          farmaco = false;
        }
        if (evacuar == true) {
          //lcd.clear();
          //lcd.setCursor(0, 0);
          //lcd.print("EVACUAR");
          hora_evacuar = printLocalTime();
          Serial.println(hora_evacuar);
          attachInterrupt(digitalPinToInterrupt(boton_evacuar), isr3, RISING);
          evacuar = false;
        }
        if (orinar == true) {
          //lcd.clear();
          //lcd.setCursor(0, 0);
          //lcd.print("ORINAR");
          hora_orinar = printLocalTime();
          Serial.println(hora_orinar);
          attachInterrupt(digitalPinToInterrupt(boton_orinar), isr4, RISING);
          orinar = false;
        }


        http.addHeader("Content-Type", "application/json");

        String jsonX = "{\"temperatura\":" + String(temp) +",\"co2\":" + String(co2) +",\"frecuencia_cardiaca\":" + String(HR) +",\"saturacion\":" + String(spo2) +",\"evacuar\":" + hora_evacuar +",\"orina\":" + hora_orinar +",\"farmaco\":" + hora_farmaco +",\"alimento\":" + hora_alimento + "}";
       
        int httpResponseCode = http.POST(jsonX);
        if (httpResponseCode > 0) {
          String response = http.getString();
          if (response == "true"){
            http.end();
            serverName = "http://69.164.216.230:3000/actualizacion1";
            WiFi.mode(WIFI_STA);
            HTTPClient client;
            client.begin(serverName);
            int resp = client.GET();
            if(resp == 200){
              // get length of document (is -1 when Server sends no Content-Length header)
              totalLength = client.getSize();
              // transfer to local variable
              int len = totalLength;
              // this is required to start firmware update process
              Update.begin(UPDATE_SIZE_UNKNOWN);
              Serial.printf("FW Size: %u\n",totalLength);
              // create buffer for read
              uint8_t buff[128] = { 0 };
              // get tcp stream
              WiFiClient * stream = client.getStreamPtr();
              // read all data from server
              Serial.println("Updating firmware...");
              while(client.connected() && (len > 0 || len == -1)) {
                  // get available data size
                  size_t size = stream->available();
                  if(size) {
                      // read up to 128 byte
                      int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                      // pass to function
                      updateFirmware(buff, c);
                      if(len > 0) {
                        len -= c;
                      }
                  }
                  delay(1);
              }
              client.end();
          }
          else{
            Serial.println("Cannot download firmware file. Only HTTP response 200: OK is supported. Double check firmware location #defined in HOST.");
          }
          //client.end();
  
          }
        }
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
      }
      else {
        Serial.println("WiFi Disconnected");
      }
      lastTime = millis();
      tiempo2 = millis();
      tf = tiempo2 - tiempo1;
              
      Serial.println(tf);
    }
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////
 
/////////////////////////////////////////// VOID SETUP ///////////////////////////////////////////////

void setup() {
 
  Serial.begin(115200);
  xTaskCreate(miTarea2, "mi_tarea", 10000, NULL, 1, NULL);
  Wire.begin();

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  //INICIALIZAMOS PANTALLA LCD.
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Ultimo Registro");

  //INICIALIZAMOS MAX30100.
  if (!pox.begin()) {
    Serial.println("Error: MAX30100 no logró inicializarse.");
    error_max = 1;
  } 
  else {
    Serial.println("MAX30100 inicializado correctamente.");
  }

  //INICIALIZAMOS CCS811.
  if (!mySensor.begin()) {
    Serial.println("Error: CCS811 no logró inicializarse.");
    error_ccs = 1;
  }
  else {
    Serial.println("CCS811 inicializado correctamente.");
  }

  //INICIALIZAMOS DS18B20.
  sensors.begin();
  sensors.setResolution(DS18B20_ADDR, 9);
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if (tempC == -127.00) {
    Serial.println("Error: DS18B20 no logró inicializarse.");
    error_ds1 = 1;
  }
  else {
    Serial.println("DS18B20 inicializado correctamente.");
  }

  //DEFINIMOS PINES BOTONES COMO INPUT
  pinMode(boton_alimento, INPUT);
  pinMode(boton_farmaco, INPUT);
  pinMode(boton_evacuar, INPUT);
  pinMode(boton_orinar, INPUT);

  //ASOCIAMOS UNA INTERRUPCCION A CADA BOTON
  attachInterrupt(digitalPinToInterrupt(boton_alimento), isr1, RISING);
  attachInterrupt(digitalPinToInterrupt(boton_farmaco), isr2, RISING);
  attachInterrupt(digitalPinToInterrupt(boton_evacuar), isr3, RISING);
  attachInterrupt(digitalPinToInterrupt(boton_orinar), isr4, RISING);  

  http.begin(client, serverName);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  if (!pox.begin()) {
    Serial.println("Error: MAX30100 no logró inicializarse.");
    error_max = 1;
  } 
  else {
    Serial.println("MAX30100 inicializado correctamente.");
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////// VOID LOOP //////////////////////////////////////////////
 
void loop() 
{ 
    //ACTUALIZAMOS VALORES SENSORES
    if (error_max == 0) {
        pox.update();
    }
    if (error_ccs == 0) {
        if (mySensor.dataAvailable()) {
            mySensor.readAlgorithmResults();
        }
    }

    if (millis() - tiempo_resp > 250) {
      tiempo_resp = millis();
      if (cont == 100) {
        lista_llena = true;
      }
      if (lista_llena) {
        v1.pop_back();
      }
      it = v1.begin();
      v1.insert(it, co2);
      cont += 1;
    }
    switch(estado) {
        case STATE_MEDICION:
          if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
            t1 = millis();
            if (error_max == 0) {
                Serial.print("HR:");
                HR = pox.getHeartRate();
                Serial.print(HR);
                Serial.print("bpm       ");
                Serial.print("SpO2:");
                spo2 = pox.getSpO2();
                Serial.print(spo2);
                Serial.print("%       ");
            }
            
            if (error_ccs == 0) {
                co2 = mySensor.getCO2();
                if (lista_llena) {
                  for (int i = 5; i < (v1.size() - 5); i++) {
                    if (v1[i] >= v1[i-1]){
                      if (v1[i-1] >= v1[i-2]){
                        if (v1[i-1] >= v1[i-3]){
                          if (v1[i-1] >= v1[i-4]){
                            if (v1[i] >= v1[i+1]){
                              if (v1[i+1] >= v1[i+2]){
                                if (v1[i+2] >= v1[i+3]){
                                  if (v1[i+2] >= v1[i+4]){
                                  it2 = v2.begin();
                                  v2.insert(it2, i);
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                    
                }
                if (lista_llena) {
                  for (int j = 0; j < v2.size() - 1; j++) {
                    var1 = v2[j] - v2[j+1];
                    if (var1 > 2){
                      it3 = v3.begin();
                      v3.insert(it3, var1);
                    }
                  }

                  for (int f = 0; f < v3.size(); f++) {
                    suma += v3[f];
                  }
                  promedio = suma/(v3.size());
                  frecuencia_resp = 60/((promedio*250)/1000);
                  co2 = frecuencia_resp;
                  v2.clear();
                  v3.clear();
  
                suma = 0;
                promedio = 0;
                frecuencia_resp = 0;}

                Serial.print("FR: ");
                Serial.print(co2);
                Serial.print("       ");
            }

            if (error_ds1 == 0) {
                sensors.requestTemperatures();
                Serial.print("Temp:");
                temp = sensors.getTempCByIndex(0);
                Serial.print(temp);
                Serial.println("°C");
            }
            tsLastReport = millis();
            t2 = millis();
            int tf = 0;
            tf = t2 - t1;
            Serial.print("Tiempo Loop: ");
            Serial.println(tf);
            estado = STATE_MENU;
            break;
          }
        case STATE_FOTO:
            estado = STATE_MEDICION;
            break;

        case STATE_AUDIO:
            estado = STATE_MEDICION;
            break;

        case STATE_SEND:
            break;

        case STATE_MENU:
            if (alimento2 == true) {
              //lcd.clear();
              lcd.setCursor(0, 2);
              lcd.print("ALIMENTO");
              alimento2 = false;
            }
            if (farmaco2 == true) {
              //lcd.clear();
              lcd.setCursor(0, 2);
              lcd.print("FARMACO ");
              farmaco2 = false;
            }
            if (evacuar2 == true) {
              //lcd.clear();
              lcd.setCursor(0, 2);
              lcd.print("EVACUAR ");
              evacuar2 = false;
            }
            if (orinar2 == true) {
              //lcd.clear();
              lcd.setCursor(0, 2);
              lcd.print("ORINAR  ");
              orinar2 = false;
            }
            estado = STATE_MEDICION;
            break;
        case STATE_OTA:
            break;
    }
}