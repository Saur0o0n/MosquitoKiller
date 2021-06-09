/*
** Mosquit Killer app for Aruino and handheld electric bug killers
** Last update: 2021.06.07
** by Adrian (Sauron) Siemieniak
*/
#include <Adafruit_ADS1X15.h>
#include <Adafruit_SSD1306.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <EEPROM.h>

SoftwareSerial mySoftwareSerial(10, 9); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
const byte mp3_1 = 14;  // start
const byte mp3_2 = 13;  // stop
const byte mp3_3 = 10; // victory
const byte mp3_4 = 0;  // go one move your ass (when there is no killing)

const byte kill_sound_cnt = 7;  // how often should hear kill sound (random(kill_sound_cnt))
// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

const byte buttonPin = 12;
bool old_butt_state = 0;
uint8_t butt_hist = 0, butt_cnt = 0;
uint16_t mosq_kills = 0;
uint16_t high_score = 0;
uint8_t next_kill_audio = 0;
uint8_t eeprom_cell = 10;
const uint16_t eeprom_code = "12345";   // this is awqard way to find out if this is the first run at all
const uint8_t kill_grace_period = 700;  // Kill will be counted every 0,7s
uint16_t last_kill = millis();

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int16_t max_val=10;

// Record top score in eeprom
void remember_score(){

  if(mosq_kills > high_score){
    high_score=mosq_kills;
    EEPROM.put(eeprom_cell, high_score);
  }
}

// Restore top score from eeprom
void restore_score(){
uint16_t ee_tmp=0;

  if(!high_score){
    EEPROM.get(eeprom_cell+10,ee_tmp);
    if(ee_tmp==eeprom_code){  // it's not the first run - restore high score
      EEPROM.get(eeprom_cell, high_score);
    }else{      // it's the first run - initialize code and remember zero as high score
      EEPROM.put(eeprom_cell+10,eeprom_code);
      remember_score();
    }
  }
}

void power_off(){
  Serial.println("Power OFF :(");
  max_val=10;

  remember_score();

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(6,6);
  display.print("Yours:");
  display.println(mosq_kills);
  display.setTextSize(1);
  display.setCursor(6,23);
  display.print("Highest: ");
  display.println(high_score);
  display.display();
  
  myDFPlayer.playFolder(2,random(mp3_2));
}

void power_on(){
  Serial.println("Power ON!");
  myDFPlayer.playFolder(1,random(mp3_1));
  
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(9,7);
  display.println("Fight!");
  display.display();
}

void display_kills(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE); 
  display.drawRect(1, 1, 126, 30, WHITE);
  display.setCursor(6,7);
  display.print("Kills: ");
  display.println(mosq_kills);
  display.display();
//  Serial.print("audio:");
//  Serial.println(next_kill_audio);
  if(next_kill_audio<mosq_kills){
    myDFPlayer.playFolder(3,random(mp3_3));
    next_kill_audio+=random(1,kill_sound_cnt);
  }
//  Serial.print(" naudio:");
//  Serial.println(next_kill_audio);
}


// Check if the power button is pressed and debounce it
bool check_button(){
bool cur_butt_state;

  cur_butt_state = digitalRead(buttonPin);
  
  //Serial.println(cur_butt_state);
  //delay(100);


// If the button state has changed - work on it's history
  if(cur_butt_state != old_butt_state && butt_cnt==0){
    butt_hist=(byte)cur_butt_state;
    butt_cnt=1;
    //Serial.println("Start debounce");
    return old_butt_state;
  }

// We are debouncing...
  if(butt_cnt>0){
    if(butt_cnt>10){    // finish debounce
      if(butt_hist<4){
        cur_butt_state=0;
        butt_cnt=0;
      }else if(butt_hist>6){
        cur_butt_state=1;
        butt_cnt=0;
      }else{ // debounce failed
        butt_cnt=1;
        butt_hist=(byte)cur_butt_state;
        return old_butt_state;  // return previous value until manage to debounce 
      }
    }else{
      butt_hist+=cur_butt_state;
      butt_cnt++;
      return old_butt_state;  // return previous value until we debounce 
    }
  }

  if(cur_butt_state>old_butt_state){
    power_on();
  }else if(cur_butt_state<old_butt_state){
    power_off();
  }
  old_butt_state=cur_butt_state;
  
  return old_butt_state;
}


void setup(void){

  delay(1200); // this is to make my nano boot from external power (https://forum.arduino.cc/t/nano-requires-manual-reset-when-not-connected-to-computer/495587/15)
  Serial.begin(115200);
  Serial.println("Fight!");
  randomSeed(analogRead(0));
  
  next_kill_audio = random(1,kill_sound_cnt);
  
  mySoftwareSerial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.clearDisplay();
  display.setTextSize(1.6);
  display.setTextColor(WHITE); 
  display.drawRect(1, 1, 126, 30, WHITE);
  display.setCursor(7,7);
  display.println("* Mosquito Killer *");
  display.setCursor(13,17);
  display.println("by Sauron 2021r.");
  display.display();

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
//    while(true){
//      delay(0); // Code to compatible with ESP8266 watch dog.
//    }
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.volume(25);  //Set volume value. From 0 to 30

  restore_score();
  
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  pinMode(buttonPin, INPUT);
  ads.begin();
  
}

void loop(void){
int16_t results;

  if(!check_button()) return;

  /* Be sure to update this value based on the IC and the gain settings! */
  float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  //float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  results = ads.readADC_Differential_0_1();  
  if(results>max_val){
    max_val=results;
    //Serial.print(".. setting new max"); Serial.println(max_val);
    delay(100);
  }

  if(results<(max_val*0.8) && max_val>100 && (last_kill+kill_grace_period)<millis()){
    Serial.print("Kill! maxV:"); Serial.print(max_val); Serial.print(" res:"); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)");
    max_val=results;
    mosq_kills++;
    display_kills();
    last_kill = millis();
  }
}
