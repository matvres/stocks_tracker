#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


//initialize the liquid crystal library
//the first parameter is  the I2C address
//the second parameter is how many rows are on your screen
//the  third parameter is how many columns are on your screen
LiquidCrystal_I2C lcd(0x27,16,2);

const byte rxPin = 2;
const byte txPin = 3;
SoftwareSerial comms_channels(rxPin,txPin);

int a;

// Timing za LCD šaltanje
long interval = 2000;
unsigned long previousMillis = 0;
unsigned long currentMillis;
short delnica = 1;

String del1 = "AMD";
String del2 = "GD";
String del3 = "PLTR";
String del4 = "LMT";

float val1 = 0.0;
float val2 = 0.0;
float val3 = 0.0;
float val4 = 0.0;

float prev_val1 = 0.0;
float prev_val2 = 0.0;
float prev_val3 = 0.0;
float prev_val4 = 0.0;

boolean day_start = true;
float fval1;
float fval2;
float fval3;
float fval4;

float daily_change_perc = 0.0;

StaticJsonDocument<128> doc;
boolean stringComplete = false;
short fnl = 0;
char m_data;
char json_data[120];
short cnt = 0;

void setup() {
  Serial.begin(9600);
  comms_channels.begin(9600);
  delay(5000);

  // Designator prihajajočih podatkov - LED
  pinMode(5,OUTPUT);

  lcd.init();
  lcd.backlight();

}

void loop() {

  currentMillis = millis();

  while(comms_channels.available() > 0) {
    m_data = comms_channels.read();
    json_data[cnt] = m_data;
    cnt++;

    if(m_data == '\n'){
      stringComplete = true;
      fnl = cnt;
      cnt = 0;
      break;
    }
  }

  if(stringComplete){
    for(int i = 0; i < fnl; i++){
      Serial.print(json_data[i]);
    }
    digitalWrite(5,1);
    delay(100);
    digitalWrite(5,0);
    stringComplete = false;

    parseJson();
    
  }

  if(currentMillis - previousMillis > interval){

      previousMillis = currentMillis;

      lcd_display();

      delnica++;

      if(delnica > 4){
        delnica = 1;
      }
  }

}

void parseJson(){
  DeserializationError error = deserializeJson(doc, json_data);
  short j = 0;

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  for (JsonPair item : doc.as<JsonObject>()) {
    // Serial.println(item.key().c_str()); // "AMD", "GD", "PLTR", "LMT"
    // Serial.println((item.value()["price"].as<float>())); // "162.67", "250.97", "16.39", "459.58"

    switch (j) {
      case 0:
        prev_val1 = val1;
        val1 = item.value()["price"].as<float>();
        if(day_start){
          fval1 = val1;
        }
        Serial.println(del1 + ":" + val1);
        break;
      case 1:
        prev_val2 = val2;
        val2 = item.value()["price"].as<float>();
        if(day_start){
          fval2 = val2;
        }
        Serial.println(del2 + ":" + val2);
        break;
      case 2:
        prev_val3 = val3;
        val3 = item.value()["price"].as<float>();
        Serial.println(del3 + ":" + val3);
        if(day_start){
          fval3 = val3;
        }
        break;
      case 3:
        prev_val4 = val4;
        val4 = item.value()["price"].as<float>();
        Serial.println(del4 + ":" + val4);
        if(day_start){
          fval4 = val4;
        }
        break;
    }

    j++;
    
  }

  day_start = false;
}

void lcd_display(){

  float change = 0.0;

  lcd.clear();

  switch(delnica){
    case 1:
      lcd.setCursor(0,0);
      lcd.print(del1);
      lcd.setCursor(4,0);
      lcd.print(val1);
      lcd.setCursor(10,0);
      lcd.print("$");
      
      daily_change_perc = ((val1 - fval1)/fval1)*100;
      if(daily_change_perc >= 0){
        lcd.setCursor(0,1);
        lcd.print("+");
        lcd.setCursor(1,1);
        lcd.print(daily_change_perc);
      }else{
        lcd.setCursor(0,1);
        lcd.print(daily_change_perc);
      }
      lcd.setCursor(6,1);
      lcd.print("%");
      
      change = val1 - prev_val1;
      if(change >= 0){
        lcd.setCursor(8,1);
        lcd.print("+");
        lcd.setCursor(9,1);
        lcd.print(change);
      }else{
        lcd.setCursor(8,1);
        lcd.print(change);
      }
      lcd.setCursor(15,1);
      lcd.print("$");
      break;
    case 2:
      lcd.setCursor(0,0);
      lcd.print(del2);
      lcd.setCursor(3,0);
      lcd.print(val2);
      lcd.setCursor(9,0);
      lcd.print("$");
      break;
    case 3:
      lcd.setCursor(0,0);
      lcd.print(del3);
      lcd.setCursor(5,0);
      lcd.print(val3);
      lcd.setCursor(10,0);
      lcd.print("$");
      break;
    case 4:
      lcd.setCursor(0,0);
      lcd.print(del4);
      lcd.setCursor(4,0);
      lcd.print(val4);
      lcd.setCursor(10,0);
      lcd.print("$");
      break;
  }

}
