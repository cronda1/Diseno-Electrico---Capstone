#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "Base64.h"
#include <ezButton.h>
#include <Update.h>


#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

#define PIN_FLASH 15
#define PIN_CAM 14

#define SALIR 4
#define FLASH 1
#define CAMARA 2

ezButton btn_flash(PIN_FLASH);
ezButton btn_camara(PIN_CAM);

WiFiClient client;
HTTPClient http;

const char* ssid = "ronda";//"RONDALAZEN-2.4G";
const char* password = "rondance";//"RONDALAZEN1928";

const char* serverName_2 = "http://69.164.216.230:3000/foto2";  //"http://192.168.1.106:1880/update-sensor";
const char* serverName_1 = "http://69.164.216.230:3000/foto"; //const char* serverName_audio = "http://69.164.216.230:3000/audio"; 
const char* HOST_OTA = "http://69.164.216.230:3000/actualizacion1";

int estado = SALIR;
String json_img = "";
String img_encoded = "";
int httpResponseCode2;
int httpResponseCode1;

bool flash = false;

void setup() {//core 1

  btn_flash.setDebounceTime(100);
  btn_camara.setDebounceTime(100);
  pinMode(4, OUTPUT);

  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif



 WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

}


void loop() {//core 1
  

  btn_flash.loop();
  btn_camara.loop();
  

  if(btn_flash.isPressed()){
    estado = FLASH;
  }
  else if(btn_camara.isPressed()){
    estado = CAMARA;
  }


  switch(estado){
    case FLASH:{ //parte en false
      flash = !flash;
      Serial.println("FLASH");
      if(flash){ //si flash es true
        digitalWrite(4, HIGH);
      }
      else{
        digitalWrite(4,LOW);
      }
      estado = SALIR;
      break;}


    case CAMARA:{
      estado = SALIR;
      Serial.println("CAMARA");
      Serial.println(CAMARA);
      Serial.println(estado);
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb){
        ESP_LOGE(TAG, "Camera Capture Failed");
        }

      
      char *input = (char *)fb->buf;
      char output[Base64.encodedLength(3)];
      img_encoded = "";
      for (int i = 0; i < fb->len; i++){
        Base64.encode(output, (input++), 3);
        if (i%3==0){
          img_encoded += String(output);
          }
      }
      esp_camera_fb_return(fb); //IMPORTANTE, LIBERAR BUFFER
      Serial.println(img_encoded);
      
      json_img = "{\"foto\":\"image/jpge\",\"data\":\""+img_encoded+"\"}";
      img_encoded = "";
      if(flash){//foto con flash
        http.begin(client, serverName_2);
        http.addHeader("Content-Type", "application/json");
        httpResponseCode2 = http.POST(json_img);
        http.end();
        Serial.print("Enviando foto con flash, Response code: ");
        Serial.println(httpResponseCode2);
        
      }
      else{
        http.begin(client, serverName_1);
        http.addHeader("Content-Type", "application/json");
        httpResponseCode1 = http.POST(json_img);
        http.end();
        Serial.print("Enviando foto sin flash, Response code: ");
        Serial.println(httpResponseCode1);


      Serial.println("CHECKPOINT");
      

       
        
      }
        
      break;}


 
  }  
}

  

  
