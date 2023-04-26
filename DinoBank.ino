#include <LiquidCrystal_I2C.h>
int col = 15;
int row = 2;
LiquidCrystal_I2C LCD(0x3F, col, row);

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_TCS34725.h"
#include <string.h>
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_614MS, TCS34725_GAIN_1X);
float r, g, b;
uint16_t c, colorTemp, lux;
float stable[3] = {0,0,0};


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;


// CHANGE THIS TO ADD YOUR WIFI USERNAME/PASSWORD
//const char * ssid = "YuFei";
//const char * password = "yufei12345";

const char* ssid = "Xiaomi_7660";
const char* password = "spaghetti";


#define USE_SERIAL Serial
int command = 2;
int message;
int results[8] = {0,0,0,0,0,0,0,0};
String submit;
int enable_coin;
int enable_note;
int counter = 0;
String val;

int five = 15; 
int ten = 32;
int twenty = 14;
int fifty = 33;
int dollar = 27;

int note = 13;
int sound = 23;

int motorPin1 = 18;
int motorPin2 = 5;


int button = 19;
int Button_State;
String main_message;


void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  Serial.println("Entered function");
  switch (type)
  {
  case WStype_DISCONNECTED:
    USE_SERIAL.printf("[WSc] Disconnected!\n");
    break;
  case WStype_CONNECTED:
    USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
    break;
  case WStype_TEXT:
    {
      USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      String info = String((char*) payload);
      command = info[0] - '0';
      if (command == 0) {
        message = info[2] - '0';
      }
      else if (command == 1){
        enable_coin = info[2] - '0';
        enable_note = info[4] - '0';
        val = info.substring(6);
      break;
      }
    }

  case WStype_BIN:
    USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
    break;
  case WStype_ERROR:
  case WStype_FRAGMENT_TEXT_START:
  case WStype_FRAGMENT_BIN_START:
  case WStype_FRAGMENT:
  case WStype_FRAGMENT_FIN:
  case WStype_PING:
  case WStype_PONG:
    Serial.println(type);
    break;
  default:
    Serial.println(type);
  }
}


void setup()
{
  //Set the baud rate
  USE_SERIAL.begin(115200);
  USE_SERIAL.printf("Begin websocket client program....");

  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--)
  {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP(ssid, password);

  // WiFi.disconnect();
  USE_SERIAL.printf("Connecting");
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    USE_SERIAL.printf(".");
    delay(100);
  }
  USE_SERIAL.printf("Connected!");

  // server address, port and URL
//  webSocket.begin("172.20.10.4", 8080, "/hardware");
  webSocket.begin("192.168.31.55", 8080,"/hardware");

  // event handler
  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);

  pinMode(five, INPUT);
  pinMode(ten, INPUT);
  pinMode(twenty, INPUT);
  pinMode(fifty, INPUT);
  pinMode(dollar, INPUT);
  pinMode(note, INPUT);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(sound, OUTPUT);
  digitalWrite(sound, HIGH);
  pinMode(button, INPUT_PULLUP);


  //tcs.setInterrupt(false);
  delay(5000);
  tcs.getRGB(&r, &g, &b);
  //tcs.setInterrupt(true);
  colorTemp = tcs.calculateColorTemperature(r, g, b);
  lux = tcs.calculateLux(r, g, b);
  stable[0] = r;
  stable[1] = g;
  stable[2] = b;
 
  LCD.init();
  LCD.backlight();
}


// This will go into loop
void loop()
{
    int five_value = digitalRead(five);
    int ten_value = digitalRead(ten);
    int twenty_value = digitalRead(twenty);
    int fifty_value = digitalRead(fifty);
    int dollar_value = digitalRead(dollar);
    int note_value = digitalRead(note);
   
    
    Button_State = digitalRead(button);
  
    if(counter==0 && command==1){
      if (enable_coin==2 && enable_note==0){
        main_message = "Insert only 1 note to match $" + val;
      }
      if (enable_coin==2 && enable_note==1){
        main_message = "Insert only notes to match $" + val;
      }
      if (enable_coin==0 && enable_note==2){
        main_message = "Insert only 1 coin to match $" + val;
      }
      if (enable_coin==1 && enable_note==2){
        main_message = "Insert only coins to match $" + val;
      }
      if (enable_coin!=2 && enable_note!=2){
        main_message = "Insert coins and notes to match $" + val;
      }
      Serial.println(main_message);
      LCD.print(main_message);
      delay(3000);
      LCD.clear();
      counter+=1;
    }
    
    if(command==1){
      if(note_value == 0 && enable_note!=2){
        Serial.println("Note mode activated");
        delay(1000);
        
        digitalWrite(motorPin1, HIGH);
        digitalWrite(motorPin2, LOW);
        analogWrite(motorPin1, 215);
        delay(1500);
  
        digitalWrite(motorPin1, LOW);
        digitalWrite(motorPin2, LOW);
        analogWrite(motorPin1, 0);
        delay(1000);
        
        while(1){
          tcs.setInterrupt(false);// turn on LED
          delay(60);  // takes 50ms to read
          tcs.getRGB(&r, &g, &b);
          tcs.setInterrupt(true);  // turn off LED
          colorTemp = tcs.calculateColorTemperature(r, g, b);
          lux = tcs.calculateLux(r, g, b);
          
          if(b>=(stable[2]+1.5)){ //r<=(stable[0]-2) && b>=(stable[2]+1.5) && g<=(stable[1]-1)
            Serial.println("2 dollars detected");
            LCD.print("2 dollars detected");
            delay(1000);
            LCD.clear();
            int intermediate = results[5]+1;
            results[5] = intermediate;
            digitalWrite(motorPin1, HIGH);
            digitalWrite(motorPin2, LOW);
            analogWrite(motorPin1, 215);
            delay(1500);
        
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, LOW);
            analogWrite(motorPin1, 0);
            delay(1000);
            break;
          }
            
          if(g>=(stable[1]+2) && r<= (stable[0]-3)){
            Serial.println("5 Dollars detected");
            LCD.print("5 Dollars detected");
            delay(1000);
            LCD.clear();
            int intermediate = results[6]+1;
            results[6] = intermediate;
            digitalWrite(motorPin1, HIGH);
            digitalWrite(motorPin2, LOW);
            analogWrite(motorPin1, 215);
            delay(1500);
        
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, LOW);
            analogWrite(motorPin1, 0);
            delay(1000);
            break;
          }
            
          if(r>=(stable[0]+4) && g<=(stable[1]-3)){
            Serial.println("10 Dollars detected");
            LCD.print("10 Dollars detected");
            delay(1000);
            LCD.clear();
            int intermediate = results[7]+1;
            results[7] = intermediate;
            digitalWrite(motorPin1, HIGH);
            digitalWrite(motorPin2, LOW);
            analogWrite(motorPin1, 215);
            delay(1500);
        
            digitalWrite(motorPin1, LOW);
            digitalWrite(motorPin2, LOW);
            analogWrite(motorPin1, 0);
            delay(1000);
            break;
          }
        }
      }
      
      if(five_value==0 && enable_coin!=2){
        Serial.println("5 cents detected");
        digitalWrite(sound, LOW);
        int intermediate = results[0]+1;
        results[0] = intermediate;
        LCD.print("5 cents detected");
        delay(1000);
        LCD.clear();
        digitalWrite(sound, HIGH);
      }
      
      if(ten_value==0 && enable_coin!=2){
        Serial.println("10 cents detected");
        digitalWrite(sound, LOW);
        int intermediate = results[1]+1;
        results[1] = intermediate;
        LCD.print("10 cents detected");
        delay(1000);
        LCD.clear();
        digitalWrite(sound, HIGH);
      }
      
      if(twenty_value==0 && enable_coin!=2){
        Serial.println("20 cents detected");
        digitalWrite(sound, LOW);
        int intermediate = results[2]+1;
        results[2] = intermediate;
        LCD.print("20 cents detected");
        delay(1000);
        LCD.clear();
        digitalWrite(sound, HIGH);
      }
      
      if(fifty_value==0 && enable_coin!=2){
        Serial.println("50 cents detected");
        digitalWrite(sound, LOW);
        int intermediate = results[3]+1;
        results[3] = intermediate;
        LCD.print("50 cents detected");
        delay(1000);
        LCD.clear();
        digitalWrite(sound, HIGH);
      }
      
      if(dollar_value==0 && enable_coin!=2){
        Serial.println("One dollar detected");
        digitalWrite(sound, LOW);
        int intermediate = results[4]+1;
        results[4] = intermediate;
        LCD.print("One dollar detected");
        delay(1000);
        LCD.clear();
        digitalWrite(sound, HIGH);
      }
    
      if (Button_State == LOW && command==1){ //Can get rid of command==1
        Serial.println("Submitting results!");
        for (int i = 0; i < 7; i++) {
        submit += String(results[i]);
        submit += ","; 
        }
        submit += String(results[7]);
        webSocket.sendTXT(submit);
        Serial.println(submit);
        command = 2;
        enable_coin = 5;
        enable_note = 5;
        counter = 0;
        submit.clear();
        memset(results,0,sizeof(results));
        delay(1000);
      }
    }
    if(command==0 && message==0){
      Serial.println("Congradulations! You got it correct!");
      LCD.print("Congradulations! You got it correct!");
      delay(1000);
      LCD.clear();
      command = 2;
    }
    if(command==0 && message==1){
      Serial.println("Wrong! Please try again!");
      LCD.print("Wrong! Please try again!");
      delay(1000);
      LCD.clear();
      command = 2;
    }
  webSocket.loop();
}
