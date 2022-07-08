/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com  
*********/

// Import required libraries
#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SparkFunCCS811.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"
//#include <ezButton.h>

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 4
#define CCS811_ADDR 0x5A
//#define DEBOUNCE_TIME 50

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
CCS811 mySensor(CCS811_ADDR);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

PulseOximeter pox;

// Variables to store temperature values
String temperatureC = "";
String CO2 = "";
String SPO2 = "";
String HR = "";
String ECG = "";
//String Evacuar = "";
//String Orina = "";
//String Farmaco = "";
//String Alimento = "";


///////////////////////////////////////
///////////////////////////////////////


// Timer variables
unsigned long lastTime = 0;  
unsigned long timerDelay = 1000;

// Replace with your network credentials
const char* ssid = "Loreto";  //AndroidAP5337
const char* password = "loreto33";  //villavilla

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  } else {
    Serial.print("Temperature Celsius: ");
    Serial.println(tempC); 
  }
  return String(tempC);
}



const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Medicina Remota</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">CO2</span>
    <span id="CO2">%CO2%</span>
    <sup class="units">&deg;ppm</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">SPO2</span>
    <span id="SPO2">%SPO2%</span>
    <sup class="units">&deg;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">HR</span>
    <span id="HR">%HR%</span>
    <sup class="units">&deg;bpm</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">ECG</span>
    <span id="ECG">%ECG%</span>
    <sup class="units">&deg;V</sup>
  </p>
  <h2>Actividad Diaria</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Evacuar</span> 
    <span id="Evacuar">%Evacuar%</span>
    <sup class="units">&deg;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Orina</span>
    <span id="Orina">%Orina%</span>
    <sup class="units">&deg;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Farmaco</span>
    <span id="Farmaco">%Fármaco%</span>
    <sup class="units">&deg;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Alimento</span>
    <span id="Alimento">%Alimento%</span>
    <sup class="units">&deg;</sup>
  </p>
  
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 1000) ;
/////
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("CO2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/CO2", true);
  xhttp.send();
}, 1000) ; //tiempo en mandar a la pagina
////
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("SPO2").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/SPO2", true);
  xhttp.send();
}, 1000) ; //tiempo en mandar a la pagina
////
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("HR").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/HR", true);
  xhttp.send();
}, 1000) ; //tiempo en mandar a la pagina
////
////
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("ECG").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/ECG", true);
  xhttp.send();
}, 1000) ; //tiempo en mandar a la pagina
////
</script>
</html>)rawliteral";

// Replaces placeholder with DS18B20 values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATUREC"){
    return temperatureC;
  }
  else if(var == "CO2"){
    return CO2;
  }
  else if (var == "SPO2"){
    return SPO2;
  }
  else if (var == "HR"){
    return HR;
  }
  else if (var == "ECG"){
    return ECG;
  }

  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(9600);
  Serial.println();
  
  // Start up the DS18B20 library
  

 //////////////// CO2 ///////////////////////////////////////
 Wire.begin();
  if (mySensor.begin() == false)
  {
    Serial.print("CCS811 error. Please check wiring. Freezing...");
    while (1)
      ;
  }
  CO2 = "0";
  SPO2 = "0";
  HR = "0";
  ECG = "0";
  
  //////////// TEMPERATURA  /////////////////////
  sensors.begin();
  temperatureC = readDSTemperatureC();


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  server.on("/CO2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", CO2.c_str());
  });
  server.on("/SPO2", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", SPO2.c_str());
  });
  server.on("/HR", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", HR.c_str());
  });
  server.on("/ECG", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ECG.c_str());
  });

  // Start server
  server.begin();

  //////////////// SPO2 ///////////////////////////////////////
  if (!pox.begin()){
    Serial.println("Failed");
    for(;;);
  }
  else{
    Serial.println("Success");
  }
}

  
 
void loop()
{
  pox.update();
  if (mySensor.dataAvailable())
  {
    mySensor.readAlgorithmResults();
  }
  
  if ((millis() - lastTime) > timerDelay) {
    temperatureC = readDSTemperatureC();
    SPO2 = String(pox.getSpO2());
    HR = String(pox.getHeartRate());
    Serial.print("SPO2: ");
    Serial.println(pox.getSpO2());
    Serial.print("HR: ");
    Serial.println(pox.getHeartRate());
    CO2 = String(mySensor.getCO2());// FUNCION QUE ONTIENE CO2 COMO STRING 

    lastTime = millis();
      
    }  
}
