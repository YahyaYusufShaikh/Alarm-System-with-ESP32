
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include <SPIFFS.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
  #include <Hash.h>
  #include "time.h"

  #include <FS.h>
#endif
#include <ESPAsyncWebServer.h>

const char* ntpServer = "in.pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 19800;//GMT+5:30



AsyncWebServer server(80);

int led_p = 2;

// REPLACE WITH YOUR NETWORK CREDENTIALS
const char* ssid = "Wifi_name";
const char* password = "Password";

const char* PARAM_STRING = "inputString";
const char* PARAM_INT = "inputInt";
const char* PARAM_INT1 = "inputInt1";
const char* PARAM_FLOAT = "inputFloat";

// HTML web page to handle 3 input fields (inputString, inputInt, inputFloat)
const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML><html><head>
 
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  
  <script>
  const inputstring = document.getElementById('inps');
  const displayS = document.getElementById('display');
    function submitMessage() {
      alert("Saved value to ESP SPIFFS");
      setTimeout(function(){ document.location.reload(false); }, 500);   
      const inputvalue = inputstring.value;
      // if(inputvalue == "Yahya"){
      //   displayS.innerHTML = "LED is ON"
      // }
      // else{
      //   displayS.innerHTML = "LED OFF"
 
      // }

    }
  </script>
   <style>
   body{
    backgroud-color = "#000000"
   }
  .card {
      width: 300px;
      border: 1px solid #ccc;
      border-radius: 5px;
      box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
      margin: 20px;
      padding: 20px;
      background-color: #fff;
       max-width: 400px;
      margin: 0 auto;
    }
  </style>
  </head><body>
   <div class="card">
  <form action="/get" target="hidden-form">
    inputString (current value %inputString%): <input type="text" id="inps" name="inputString">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form><br>
  <span id="display"></span>
  <form action="/get" target="hidden-form">
  <div class="form-container1">
    Minute (current value %inputInt%): <input type="number " class="form-input1" name="inputInt">
    <input type="submit" value="Submit" class="form-submit1" onclick="submitMessage()">
    </div>
  </form><br>
 <form action="/get" target="hidden-form">
 <div class="form-container2">
     Hour (current value %inputInt1%): <input type="number " class="form-input2" name="inputInt1">
     <input type="submit" value="Submit" class="form-submit2" onclick="submitMessage()">
     </div>
   </form><br>
  <form action="/get" target="hidden-form">
    inputFloat (current value %inputFloat%): <input type="number " name="inputFloat">
    <input type="submit" value="Submit" onclick="submitMessage()">
  </form>
  <iframe style="display:none" name="hidden-form"></iframe>
  </div>
 </body></html>
)rawliteral";

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

String readFile(fs::FS &fs, const char * path){
  //Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if(!file || file.isDirectory()){
    Serial.println("- empty file or failed to open file");
    return String();
  }
  //Serial.println("- read from file:");
  String fileContent;
  while(file.available()){
    fileContent+=String((char)file.read());
  }
  file.close();
  //Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
  file.close();
}

// Replaces placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  if(var == "inputString"){
    return readFile(SPIFFS, "/inputString.txt");
  }
  else if(var == "inputInt"){
    return readFile(SPIFFS, "/inputInt.txt");
  }
  else if(var == "inputInt1"){
    return readFile(SPIFFS, "/inputInt1.txt");
  }
  else if(var == "inputFloat"){
    return readFile(SPIFFS, "/inputFloat.txt");
  }
  return String();
}
int day ;
int hour;
int Min ;
int Sec ;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
day  =  timeinfo.tm_wday;
hour = timeinfo.tm_hour;
Min  =  timeinfo.tm_min;
Sec  = timeinfo.tm_sec;
}


void setup() {

pinMode(led_p, OUTPUT);

  Serial.begin(115200);
  // Initialize SPIFFS
  #ifdef ESP32
    if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #else
    if(!SPIFFS.begin()){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
  #endif

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed!");
    return;
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      writeFile(SPIFFS, "/inputString.txt", inputMessage.c_str());
    }
    // GET inputInt value on <ESP_IP>/get?inputInt=<inputMessage>
    else if (request->hasParam(PARAM_INT)) {
      inputMessage = request->getParam(PARAM_INT)->value();
      writeFile(SPIFFS, "/inputInt.txt", inputMessage.c_str());
    }
      else if (request->hasParam(PARAM_INT1)) {
      inputMessage = request->getParam(PARAM_INT1)->value();
      writeFile(SPIFFS, "/inputInt1.txt", inputMessage.c_str());
    }
    // GET inputFloat value on <ESP_IP>/get?inputFloat=<inputMessage>
    else if (request->hasParam(PARAM_FLOAT)) {
      inputMessage = request->getParam(PARAM_FLOAT)->value();
      writeFile(SPIFFS, "/inputFloat.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();


 configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();



}

void loop() {

    printLocalTime();

  // To access your stored values on inputString, inputInt, inputFloat
  String yourInputString = readFile(SPIFFS, "/inputString.txt");
  // Serial.print("*** Your inputString: ");
  // Serial.println(yourInputString);


  // if(yourInputString == "Yahya"){
  //   digitalWrite(led_p, HIGH);
  // }
  // else{
  //   digitalWrite(led_p, LOW);
  // }


  int yourInputInt = readFile(SPIFFS, "/inputInt.txt").toInt();
  // Serial.print("*** Your inputInt: ");
  // Serial.println(yourInputInt);
  
 int yourInputInt1 = readFile(SPIFFS, "/inputInt1.txt").toInt();
  // Serial.print("*** Your inputInt1: ");
  // Serial.println(yourInputInt1);

   if(yourInputInt == Min && yourInputInt1 == hour){
    digitalWrite(led_p, HIGH);
  }
  else{
    digitalWrite(led_p, LOW);
  }

  float yourInputFloat = readFile(SPIFFS, "/inputFloat.txt").toFloat();
  // Serial.print("*** Your inputFloat: ");
  // Serial.println(yourInputFloat);
  delay(500);
}