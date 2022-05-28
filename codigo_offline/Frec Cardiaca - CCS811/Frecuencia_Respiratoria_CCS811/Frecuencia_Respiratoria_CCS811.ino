/******************************************************************************
  Read basic CO2 and TVOCs

  Marshall Taylor @ SparkFun Electronics
  Nathan Seidle @ SparkFun Electronics

  April 4, 2017

  https://github.com/sparkfun/CCS811_Air_Quality_Breakout
  https://github.com/sparkfun/SparkFun_CCS811_Arduino_Library

  Read the TVOC and CO2 values from the SparkFun CSS811 breakout board

  A new sensor requires at 48-burn in. Once burned in a sensor requires
  20 minutes of run in before readings are considered good.
******************************************************************************/
#include <Wire.h>

#include "SparkFunCCS811.h" 

#define CCS811_ADDR 0x5A // La I2C Address por default (0x5B) no funicionaba. En cambio, la I2C Address en 0x5A logró hacer funcionar la comunicación con el sensor.

CCS811 mySensor(CCS811_ADDR);

int cont = 0;
int CO2 = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println("CCS811 Frecuencia Respiratoria");

  Wire.begin(); //Inicializando I2C.

  if (mySensor.begin() == false)
  {
    Serial.print("CCS811 error. Revisar conecciones...");
    while (1);
  }
}

void loop()
{
  //Revisamos si hay data disponible con .dataAvailable()
  if (mySensor.dataAvailable())
  {
    //Se confirma que hay data, para obtener las lecturas primero tenemos que leerlas.
    mySensor.readAlgorithmResults();

    // Ahora las obtenemos e imprimimos en el monitor serial.
    Serial.print("CO2[");
    CO2 = mySensor.getCO2();
    Serial.print(CO2);
    Serial.print("]  ");
    Serial.print("millis[");
    Serial.print(millis());
    Serial.print("] ");

    if (cont != 0)
    {
      cont = cont - 1;
    }

    if ((CO2 > 1000) && (cont = 0))
    {
      cont = 3;
      Serial.print("Exhalando...");
    }

    Serial.println();
  }

  delay(10); 
}
