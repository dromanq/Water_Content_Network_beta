/*
 BRIDGE IN NODE 1
 Upload to ThingSpeak
*/

///////////INCLUDE LIBRARIES

#include <FS.h>
#include <SPI.h>
//#include <ESP8266WiFi.h>
#include <WiFi.h>
//#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <stdlib.h>
WiFiClient Client;
#include <ESP32Time.h>
ESP32Time rtc;

///////////DEFINE SERIAL COM PINS

#define RXD2 16
#define TXD2 17

///////////DEFINE WIFI CREDENTIALS

//String ssid = "Redmi Note 9S";
//String password = "123456789";

//String ssid = "TIM-28464841_2.4";
//String password = "43BE354815";

/*const char * ssid = "TIM-28464841_2.4";
const char * password = "43BE354815"; //THE ESP32 USE CHAR*/

/*String ssid = "4G-UFI-1203";
String password = "1234567890"; //THE ESP8266 USE STRING*/

const char * ssid = "4G-UFI-1203";
const char * password = "1234567890"; //THE ESP32 USE CHAR*/

//String ssid = "Lab_Geotecnica";
//String password = "labgeo22!";

///////////DEFINE THINGSPEAK CREDENTIALS

unsigned long myChannelNumber = 1900126;
const char * myWriteAPIKey = "NW0W622KU01YG7HD";

//DEFINE SLEEP PARAMETERS
    
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 3300         /* Time ESP32 will go to sleep (in seconds) */

//DEFINE GLOBAL VARIABLES

int node = 2;
int LED = 14;
//int dt = 1000;
String Rxdchar;
bool msgReady = false;

byte cont = 0;
byte max_try = 50;

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

RTC_DATA_ATTR int bootCount = 0; //Save cycle variables in RTC memory

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
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
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
  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);
  pinMode(LED, OUTPUT);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  ThingSpeak.begin(Client);

  while (WiFi.status() != WL_CONNECTED and cont < max_try){
    cont++;
    digitalWrite(LED, LOW);
    delay(150);
    digitalWrite(LED, HIGH);
    delay(150);    
    }

  if(WiFi.status() == WL_CONNECTED) {
    digitalWrite(LED, LOW);
    }
  
  segundo_ini = rtc.getTime("%S");
  i_segundo_ini = segundo_ini.toInt();
  //Serial.println(segundo_ini); Serial.println(segundo_ini.toInt());
  minuto_ini = rtc.getTime("%M");
  i_minuto_ini = minuto_ini.toInt();    
  }

void loop() { //Choose Serial1 or Serial2 as required  
  
  digitalWrite(LED, LOW);
  delay(100);
  digitalWrite(LED, HIGH);
  delay(100);

  if (WiFi.status() != WL_CONNECTED){
    delay(20000);
    Serial.println("WiFi disconnected");
    ESP.restart();
    }
  
  while (Serial.available() > 0) {
    //Serial.println("enter loop");
    Rxdchar = Serial.readString();    
    Serial.print(Rxdchar);
    msgReady = true;    
    }
  
  if (msgReady) {
    int j = 0;
    for (int i = 0;i < 6;i++){
      String inVar;
      while (Rxdchar[j] != '\n'){
        inVar = inVar + Rxdchar[j];
        //Serial.print(inVar);
        j++;
        delay(500);
        //Serial.println("\n");
        }
      float finVar = inVar.toFloat();
      
      int httpCode = ThingSpeak.writeField(myChannelNumber, i + 1, finVar, myWriteAPIKey);
      delay(20000);           
      j++;
      }
    msgReady = false;
        
    }
    
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

    if (delta_minuto >= 180) {
      Serial.println("Going to sleep");
      esp_deep_sleep_start();
      }  
  }
