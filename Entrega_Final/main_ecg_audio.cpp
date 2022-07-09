#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>
#include <SPIFFS.h>
#include <import.h>



#define pushButton_pin   12
#define DEBOUNCE_TIME 100
#define ECG_pin 35 //VP
#define ECG_BUFFER_MAX 500

#define I2S_WS 33
#define I2S_SD 32
#define I2S_SCK 25
#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE   (16000)
#define I2S_SAMPLE_BITS   (16)
#define I2S_READ_LEN      (16 * 1024)
#define RECORD_TIME       (5) //Seconds
#define I2S_CHANNEL_NUM   (1)
#define FLASH_RECORD_SIZE (I2S_CHANNEL_NUM * I2S_SAMPLE_RATE * I2S_SAMPLE_BITS / 8 * RECORD_TIME)

#define SEND_ECG 2
#define SEND_AUDIO 3

void IRAM_ATTR isr(void);
bool debounce(int);
void miTarea2 (void *pvParameters);

void SPIFFSInit(void);
void listSPIFFS(void);
void i2sInit(void);
void i2s_adc(void *arg);
void i2s_adc_data_scale(uint8_t * d_buff, uint8_t* s_buff, uint32_t len);
void wavHeader(byte* header, int wavSize);


//VARIABLES GLOBALES
bool in = false;
int count = 0;
int send = 0;
String audio_encoded;
File file;
File file2;
const char filename[] = "/recording.wav";
const int headerSize = 44;



//ECG
bool interrupt = false;
String ecg_send;
String ecg_buffer = "";
int pos = 0;



WiFiClient  client;
HTTPClient http;
//VARIABLES COMUNICACION SERVIDOR 
const char* ssid = "ronda";    //"Loreto";    //AndroidAP5337   //"Trini's Galaxy Z Flip"
const char* password = "rondance";           //"loreto33";  //villavilla
const char* serverName_ecg = "http://69.164.216.230:3000/ecg";
const char* serverName_audio = "http://69.164.216.230:3000/audio";

//TIMER
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interrupt = true;
  portEXIT_CRITICAL_ISR(&timerMux);
}


void setup() { //CORE 1
  // put your setup code here, to run once:
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(pushButton_pin), isr, FALLING); 

  timer = timerBegin(0, 80, true); //cuenta hasta 1 000 000
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 4000, true); //4000 son 250 hz
  timerAlarmEnable(timer);

  xTaskCreate(miTarea2, "mi_tarea", 10000, NULL, 1, NULL);
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  http.begin(client, serverName_ecg);

  i2sInit();
  
  
}
int contador = 0;
void loop() { //CORE 1
  // put your main code here, to run repeatedly:
  if(interrupt){//de timer
    ecg_buffer += (String)analogRead(ECG_pin);
    ecg_buffer += ", ";
    pos += 1;
    interrupt = false;
    if(pos >= ECG_BUFFER_MAX){
      ecg_buffer.remove(ecg_buffer.length()-2);
      //MANDAR A SERVIDOR
      ecg_send = ecg_buffer;
      ecg_buffer = "";
      pos = 0;
      send = SEND_ECG;
      Serial.print(int(ECG_BUFFER_MAX));
      Serial.println(" muestras obtenidas");
     
      }

    }

  if(in){
    timerAlarmDisable(timer);
    if(debounce(millis())){

      SPIFFSInit();
      i2s_start(I2S_PORT);  
      int i2s_read_len = I2S_READ_LEN;
      int flash_wr_size = 0;
      size_t bytes_read;

      char* i2s_read_buff = (char*) calloc(i2s_read_len, sizeof(char));
      uint8_t* flash_write_buff = (uint8_t*) calloc(i2s_read_len, sizeof(char));

      i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
      i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
      //ESOS I2S>_READ ELIMINAN UN POCO EL RUIDO
      Serial.println(" *** Recording Start *** ");
      while (flash_wr_size < FLASH_RECORD_SIZE) {
          //read data from I2S bus, in this case, from ADC.
          i2s_read(I2S_PORT, (void*) i2s_read_buff, i2s_read_len, &bytes_read, portMAX_DELAY);
          //example_disp_buf((uint8_t*) i2s_read_buff, 64);
          //save original data from I2S(ADC) into flash.
          i2s_adc_data_scale(flash_write_buff, (uint8_t*)i2s_read_buff, i2s_read_len);
          file.write((const byte*) flash_write_buff, i2s_read_len);
          flash_wr_size += i2s_read_len;
          ets_printf("Sound recording %u%%\n", flash_wr_size * 100 / FLASH_RECORD_SIZE);
          ets_printf("Never Used Stack Size: %u\n", uxTaskGetStackHighWaterMark(NULL));
          //audio_in = false;
      }
      i2s_stop(I2S_PORT);
      file.close();

      free(i2s_read_buff);
      i2s_read_buff = NULL;
      free(flash_write_buff);
      flash_write_buff = NULL;
      
      listSPIFFS();



      send = SEND_AUDIO;
      in = false;
      attachInterrupt(digitalPinToInterrupt(pushButton_pin), isr, FALLING);
      
    }
  }

}


void IRAM_ATTR isr()
{
  in = true;
  count += 1;
  detachInterrupt(digitalPinToInterrupt(pushButton_pin));
}

bool debounce(int time){
  while((millis() - time) < DEBOUNCE_TIME){}
  return true;
}

void miTarea2 (void *pvParameters){
  for(;;){
    vTaskDelay(100);
    switch(send){
      case SEND_AUDIO:{
      Serial.println("mandando audio");
      /*http.end(); //desconectamos de ECG
      http.begin(client, serverName_audio);
      const char *fname = "/spiffs/recording.wav";
      FILE *fp = fopen(fname, "rb");
      fseek(fp, 0, SEEK_END); //send file pointer to end of file 
      int file_size = ftell(fp); //get end position of file //163884
      fseek(fp, 0, SEEK_SET); //send pointer back to start
      Serial.println(file_size);

      int max_upload_size = 5000; // array size, larger = less uploads but too large can cause memory issues
      int num_of_uploads = file_size / max_upload_size; // figure out how many evenly sized upload chunks we need
      int num_of_uploads_mod = file_size % max_upload_size; //find out size of remaining upload chunk if needed
      Serial.println(num_of_uploads);
      int i;


      //upload file in even chunks    
      if (num_of_uploads > 0)
      {
        uint8_t buff1[max_upload_size] = {}; // array to save file too. add 1 for end of array symbol '\n'
        for (i = 0; i < num_of_uploads; i++)
        {
          fread(buff1, sizeof *buff1, sizeof buff1 / sizeof *buff1, fp); // -1 as don't want to count the '\n'
          http.addHeader("File_name", "test file"); //header to say what the file name is
          http.addHeader("Content-Type", "application/octet-stream");
          int httpResponseCode = http.POST((uint8_t *)buff1, sizeof(buff1)); //send data. Datatype is (uint8_t *)
          Serial.println("etapa 1");
        }
      }
      num_of_uploads = 0;
      //upload any remaining data
      if (num_of_uploads_mod > 0)
      {
        int remainder = file_size - num_of_uploads * max_upload_size;
        uint8_t buff2[remainder] = {};
        fread(buff2, sizeof *buff2, sizeof buff2 / sizeof *buff2, fp); //read from file and store to buff2
        http.addHeader("File_name", "test file");
        http.addHeader("Content-Type", "application/octet-stream");
        int httpResponseCode = http.POST(buff2, sizeof(buff2));
        Serial.println("etapa 2");
      }
      num_of_uploads_mod = 0;
      fclose(fp);
      http.end(); // Close connection
      http.begin(client, serverName_ecg);
      delay(10 * 1000);
      send = 0;
      timerAlarmEnable(timer);*/
      break;}

      case SEND_ECG:{
      Serial.println("ENVIANDO ECG");
      http.addHeader("Content-Type", "application/json");
      String json_ecg = "{\"ecg\":\"" + ecg_send +  "\"}";
      int httpResponseCode = http.POST(json_ecg);
      Serial.println(httpResponseCode);
      //Serial.println(json_ecg); //ARREGLAR TIMEOUT
      //Serial.println("enviando ECG");
      send = 0;
      break;}
    }
  

    }

  }

void wavHeader(byte* header, int wavSize){
  header[0] = 'R';
  header[1] = 'I';
  header[2] = 'F';
  header[3] = 'F';
  unsigned int fileSize = wavSize + headerSize - 8;
  header[4] = (byte)(fileSize & 0xFF);
  header[5] = (byte)((fileSize >> 8) & 0xFF);
  header[6] = (byte)((fileSize >> 16) & 0xFF);
  header[7] = (byte)((fileSize >> 24) & 0xFF);
  header[8] = 'W';
  header[9] = 'A';
  header[10] = 'V';
  header[11] = 'E';
  header[12] = 'f';
  header[13] = 'm';
  header[14] = 't';
  header[15] = ' ';
  header[16] = 0x10;
  header[17] = 0x00;
  header[18] = 0x00;
  header[19] = 0x00;
  header[20] = 0x01;
  header[21] = 0x00;
  header[22] = 0x01;
  header[23] = 0x00;
  header[24] = 0x80;
  header[25] = 0x3E;
  header[26] = 0x00;
  header[27] = 0x00;
  header[28] = 0x00;
  header[29] = 0x7D;
  header[30] = 0x00;
  header[31] = 0x00;
  header[32] = 0x02;
  header[33] = 0x00;
  header[34] = 0x10;
  header[35] = 0x00;
  header[36] = 'd';
  header[37] = 'a';
  header[38] = 't';
  header[39] = 'a';
  header[40] = (byte)(wavSize & 0xFF);
  header[41] = (byte)((wavSize >> 8) & 0xFF);
  header[42] = (byte)((wavSize >> 16) & 0xFF);
  header[43] = (byte)((wavSize >> 24) & 0xFF);
  
}

void listSPIFFS(void) {
  Serial.println(F("\r\nListing SPIFFS files:"));
  static const char line[] PROGMEM =  "=================================================";

  Serial.println(FPSTR(line));
  Serial.println(F("  File name                              Size"));
  Serial.println(FPSTR(line));

  fs::File root = SPIFFS.open("/");
  if (!root) {
    Serial.println(F("Failed to open directory"));
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(F("Not a directory"));
    return;
  }

  fs::File file = root.openNextFile();
  while (file) {

    if (file.isDirectory()) {
      Serial.print("DIR : ");
      String fileName = file.name();
      Serial.print(fileName);
    } else {
      String fileName = file.name();
      Serial.print("  " + fileName);
      // File path can be 31 characters maximum in SPIFFS
      int spaces = 33 - fileName.length(); // Tabulate nicely
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      String fileSize = (String) file.size();
      spaces = 10 - fileSize.length(); // Tabulate nicely
      if (spaces < 1) spaces = 1;
      while (spaces--) Serial.print(" ");
      Serial.println(fileSize + " bytes");
    }

    file = root.openNextFile();
  }

  Serial.println(FPSTR(line));
  Serial.println();
  delay(1000);
}

void i2s_adc_data_scale(uint8_t * d_buff, uint8_t* s_buff, uint32_t len)
{
    uint32_t j = 0;
    uint32_t dac_value = 0;
    for (int i = 0; i < len; i += 2) {
        dac_value = ((((uint16_t) (s_buff[i + 1] & 0xf) << 8) | ((s_buff[i + 0]))));
        d_buff[j++] = 0;
        d_buff[j++] = dac_value * 256 / 2048;
    }
}

void i2sInit(){
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = i2s_bits_per_sample_t(I2S_SAMPLE_BITS),
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 64,
    .dma_buf_len = 1024,
    .use_apll = 1
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);

  const i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_stop(I2S_PORT);
}

void SPIFFSInit(){
  if(!SPIFFS.begin(true)){
    Serial.println("SPIFFS initialisation failed!");
    while(1) yield();
  }

  SPIFFS.remove(filename);
  file = SPIFFS.open(filename, FILE_WRITE);
  if(!file){
    Serial.println("File is not available!");
  }

  byte header[headerSize];
  wavHeader(header, FLASH_RECORD_SIZE);

  file.write(header, headerSize);
  listSPIFFS();
}

