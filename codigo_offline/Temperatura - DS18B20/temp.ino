//Incluir las bibliotecas OneWire y DallasTemperature

#include <OneWire.h>

#include <DallasTemperature.h>

//Selecciona el pin al que se conecta el sensor de temperatura

const int oneWireBus = 4;

//Comunicar que vamos a utilizar la interfaz oneWire

OneWire oneWire(oneWireBus);

//Indica que el sensor utilizará la interfaz OneWire

DallasTemperature sensors (&oneWire);

void setup() {

  //Ajustar la velocidad para el monitor serie

  Serial.begin(115200);

  sensors.begin();

}

void loop() {

  //Leer la temperatura

  Serial.print("Mandando comandos a los sensores ");

  sensors.requestTemperatures();

  //Lectura en grados celsius

  float temperatureC = sensors.getTempCByIndex(0);

 //Escribir los datos en el monitor de serie

   Serial.print("Temperatura sensor : ");

  Serial.print(temperatureC);

  Serial.println("°C");

  // Lectura de la temperatura cada 5 segundos

  delay(5000);

}
