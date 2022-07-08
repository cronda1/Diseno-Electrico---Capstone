
#include <driver/i2s.h>

const double samplingFrequency = 44100;
const int SAMPLEBLOCK = 8;
const i2s_port_t I2S_PORT = I2S_NUM_0;

int16_t samples[SAMPLEBLOCK];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Setting up Audio Input I2S");
  setupI2S();
  Serial.println("Audio input setup completed");
  delay(1000);
}

void loop() {
    int num_bytes_read = i2s_read_bytes(I2S_PORT, 
                                      (char *)samples, 
                                      SAMPLEBLOCK,
                                      portMAX_DELAY); // no timeout
  for (uint16_t i = 0; i < SAMPLEBLOCK; i++) {
    Serial.printf("%7d,",samples[i]); 
  }
    Serial.printf("\n"); 
}  
 
void setupI2S() {
  Serial.println("Configuring I2S...");
  esp_err_t err;
  // The I2S config as per the example
  const i2s_config_t i2s_config = { 
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_ADC_BUILT_IN),
      .sample_rate = samplingFrequency,                        
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // could only get it to work with 32bits
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT, // although the SEL config should be left, it seems to transmit on right
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
      .dma_buf_count = 2,                           // number of buffers
      .dma_buf_len = SAMPLEBLOCK,                     // samples per buffer
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0
  };

 err = adc_gpio_init(ADC_UNIT_1, ADC_CHANNEL_0); //step 1
  if (err != ESP_OK) {
    Serial.printf("Failed setting up adc channel: %d\n", err);
    while (true);
  }
 
  err = i2s_driver_install(I2S_NUM_0, &i2s_config,  0, NULL);  //step 2
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }

  err = i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_0);
    if (err != ESP_OK) {
    Serial.printf("Failed setting up adc mode: %d\n", err);
    while (true);
  }
  
  Serial.println("I2S driver installed.");
}
