/*
  MESH NODE NO. 2
  INCLUDE DEEP SLEEP - Aquisition every hour
*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <ESP32Time.h>
ESP32Time rtc;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 26;

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Water content sensor is connected to GPIO 34 (Analog ADC1_CH6) 
const int WCPin = 34;

// MESH Details
#define   MESH_PREFIX     "RNTMESH" //name for your MESH
#define   MESH_PASSWORD   "MESHpassword" //password for your MESH
#define   MESH_PORT       5555 //default port

//DEFINE SLEEP PARAMETERS 
    
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 3300         /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR int bootCount = 0; //Save cycle variables in RTC memory

//Number for this node
int nodeNumber = 2;
int LED = 14; //Debugging led

//String to send to other nodes with sensor readings
String readings;
int dt = 5; //Broadcasting time in seconds

String cadena;
String separador = "\n";

bool msgSent = false;
bool msgReceived = false;
bool ready2sleep = false;
bool neverAgain = true;

//Declare global variables
double node1;
double hum1;
double temp1;

double node2;
double temp2;
double hum2;

int elapsTime;
String segundo; String segundo_i;
String minuto; String minuto_i;
int delta_minuto = 0;
int dsegundo;
int dsegundo2;
int dminuto;
int dminuto2;

String segundo_ini;
String minuto_ini;
int i_segundo_ini;
int i_minuto_ini;

String minuto_fin;
String segundo_fin;
int i_minuto_fin;
int i_segundo_fin;

int i_segundo_i;
int i_minuto_i;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings

//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND * dt , TASK_FOREVER, &sendMessage);

String getReadings () {
  sensors.requestTemperatures();
  JSONVar jsonReadings;
  jsonReadings["node"] = (analogRead(WCPin)/4095)*3.3;
  jsonReadings["temp"] = sensors.getTempCByIndex(0);
  jsonReadings["hum"] = analogRead(WCPin);
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
  msgSent = true;
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  node2 = (analogRead(WCPin)/4095)*3.3;
  hum2 = analogRead(WCPin);
  temp2 = sensors.getTempCByIndex(0);
  //Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  node1 = myObject["node"];
  temp1 = myObject["temp"];
  hum1 = myObject["hum"];
  msgReceived = true;  
}

void newConnectionCallback(uint32_t nodeId) {
  //Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  //Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setESPtime(){

  const char compile_time[] = __TIME__;
    
  String hr = String(compile_time[0]) + String(compile_time[1]);
  String mn = String(compile_time[3]) + String(compile_time[4]);
  String sc = String(compile_time[6]) + String(compile_time[7]);
  int inthr = hr.toInt(); int intmn = mn.toInt(); int intsc = sc.toInt();
  rtc.setTime(intsc, intmn, inthr, 5, 11, 2022);   
  
  ++bootCount;
  }

void setup() {
  setESPtime();

  segundo_i = rtc.getTime("%S"); segundo = segundo_i;
  i_segundo_i = segundo_i.toInt();
  minuto_i = rtc.getTime("%M"); minuto = minuto_i;
  i_minuto_i = minuto_i.toInt();
  
  while (minuto.toInt() != 0 || segundo.toInt() != 0) {    
    segundo = rtc.getTime("%S");
    minuto = rtc.getTime("%M");
    }
    
  Serial.begin(115200);
  Serial.print("I'm awake");
  pinMode(LED, OUTPUT);  

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  digitalWrite(LED_BUILTIN, LOW);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void loop() {
  // it will run the user scheduler as well
  digitalWrite(LED, HIGH);

  segundo_ini = rtc.getTime("%S");
  i_segundo_ini = segundo_ini.toInt();
  //Serial.println(segundo_ini); Serial.println(segundo_ini.toInt());
  minuto_ini = rtc.getTime("%M");
  i_minuto_ini = minuto_ini.toInt();
  //Serial.println(minuto_ini); Serial.println(minuto_ini.toInt());
      
  while (ready2sleep == false || delta_minuto < 180) {
    mesh.update();      

    minuto_fin = rtc.getTime("%M");
    segundo_fin = rtc.getTime("%S");

    i_minuto_fin = minuto_fin.toInt();
    i_segundo_fin = segundo_fin.toInt();

    if (i_segundo_fin >= i_segundo_ini){
      dsegundo = i_segundo_fin - i_segundo_ini;
      }
    else {
      dsegundo = i_segundo_fin + 60 - i_segundo_ini;
      }

    if (i_minuto_fin >= i_minuto_ini){
      dminuto = i_minuto_fin - i_minuto_ini;
      }
    else {
      dminuto = i_minuto_fin + 60 - i_minuto_ini;
      }

    delta_minuto = dminuto * 60 + dsegundo;
    
    //Serial.print(msgReceived);
  
    if (msgReceived) {      
      msgReceived = false;
      ready2sleep = true;
      if (neverAgain) {
        cadena = String(node2) + separador + String(hum2) + separador + String(temp2) + separador + String(node1) + separador + String(hum1) + separador + String(temp1) + separador; //Does not include volt
        int cadena_len = cadena.length() + 1;
        char char_cadena[cadena_len];

        cadena.toCharArray(char_cadena, cadena_len);
        Serial.print(char_cadena);
        //Serial.print(delta_minuto);
        neverAgain = false;
        }
      }
    }

  Serial.println("Operative time:");
  Serial.println(delta_minuto);

  if (ready2sleep){ //MAKE IT SLEEP
    Serial.println("Going to sleep");
    esp_deep_sleep_start();
    }  

}
