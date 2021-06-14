/*
** Mosquit Killer v1.0 app for Aruino and handheld electric bug killers
** Last update: 2021.06.14
** by Adrian (Sauron) Siemieniak
*/
#include <Adafruit_ADS1X15.h>
#include <Adafruit_SSD1306.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <EEPROM.h>

SoftwareSerial mySoftwareSerial(10, 9); // RX, TX
const byte zapButtonPin = 12;  // ping where we read zap enable button
const byte menuButtonPin1 = 8; // menu button pin

DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
const uint8_t mp3_1 = 17; // (+1) start
const uint8_t mp3_2 = 13; // (+1) stop
const uint8_t mp3_3 = 5;  // (+1) victory
const uint8_t mp3_4 = 6;  // (+1) combo sound
const uint8_t mp3_5 = 10;  // (+1) go on and move your ass sounds (when there is no killing)

const byte kill_sound_cnt = 15;  // how often should hear kill sound (random(kill_sound_cnt))
// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

bool old_zap_butt_state = 0;
uint8_t zap_butt_hist = 0, zap_butt_cnt = 0;

const uint16_t menu_button_short = 800; // how long (ms) is short press, everything above is long press
uint32_t menu_button_time = 0;  // when was last time menu button was pressed (to count long/short presses)

uint16_t mosq_kills = 0;
uint16_t high_score = 0;
uint8_t next_kill_audio = 0;
const uint16_t kill_grace_period = 700;  // Kill will be counted every 0,7s
const uint16_t kill_combo_threshold = 1500;  // window where next kill is counted for combo
uint8_t kombo_kill_count = 0; // how much killed in a row
uint32_t last_kill = millis();

uint32_t next_sentence = 0;  // if "player" is doing nothing, play something not too often

const uint32_t min_delay_for_sentence = 60000;  // Some minimal delay, like 60s
const uint32_t max_delay_for_sentence = 1200000; // And maximum delay - like 20min
/*
 * Preferences (kind off ;) )
*/
bool prefs_audio = 1; // audio on/off
int eeprom_cell = 10;
const uint16_t eeprom_code = 22345;   // this is awkward way to find out if this is the first run at all, put here unique value in range 0...65535


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

int16_t max_val=10; // max voltage value (not in volts) - reset after each hit
int16_t total_max_val=10;  // max value total - to measure charging

// Record top score in eeprom
void remember_score(){

  if(mosq_kills > high_score){
    high_score=mosq_kills;
    EEPROM.put(eeprom_cell+10, high_score);
    EEPROM.put(eeprom_cell+12, prefs_audio);
  }
}

// Restore top score from eeprom
void restore_score(){
uint16_t ee_tmp=0;

  if(!high_score){
    EEPROM.get(eeprom_cell,ee_tmp);
    if(ee_tmp==eeprom_code){  // it's not the first run - restore high score
      Serial.println("Restoring eeprom");
      Serial.print(" eeprom code was: ");Serial.println(ee_tmp);
      EEPROM.get(eeprom_cell+10, high_score);
      Serial.print(" High score value: "); Serial.println(high_score);
      EEPROM.get(eeprom_cell+12, prefs_audio);
    }else{      // it's the first run - initialize code and remember zero as high score
      Serial.println("Initializing eeprom - first run");
      Serial.print(" eeprom code was:"); Serial.println(ee_tmp);
      EEPROM.put(eeprom_cell,eeprom_code);
      
      high_score=mosq_kills=0;
      EEPROM.put(eeprom_cell+10, high_score);
      EEPROM.put(eeprom_cell+12, prefs_audio);
    
      //remember_score();
    }
  }
}

void power_off(){
static bool first=1;

  Serial.println("Power OFF :(");
  max_val=10;

  remember_score();

  display.clearDisplay();
  if(mosq_kills==high_score && high_score>0){
    if(first){  // play highscore audio only once for a session
      if(prefs_audio) myDFPlayer.playFolder(6,50);
      first=0;
    }
    display.setTextSize(2);
    display.setCursor(4,1);
    display.print("Highscore!");
    display.setCursor(16,18);
    display.print(high_score);
    display.print(" kills");
  }else{
    if(prefs_audio) myDFPlayer.playFolder(2,random(mp3_2));
    display.setTextSize(2);
    display.setCursor(6,6);
    display.print("Yours:");
    display.println(mosq_kills);
    display.setTextSize(1);
    display.setCursor(6,23);
    display.print("Highest: ");
    display.println(high_score);
  }
  display.display();
}

void power_on(){
  Serial.println("Power ON!");
  if(prefs_audio) myDFPlayer.playFolder(1,random(mp3_1));
  
  display.clearDisplay();
  display.setTextSize(3);
  display.setCursor(9,7);
  display.println("Fight!");
  display.display();
}


// Menu short press
void menu_short_press(){
  Serial.println("Short press");
}

// Process long press
void menu_long_press(){
  Serial.println("Long press");
  prefs_audio=!prefs_audio;
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(7,10);
  if(prefs_audio) display.println("Audio on");
  else display.println("Audio off");
  display.display();
  EEPROM.put(eeprom_cell+12, prefs_audio);
}

// Process menu button(s)
void check_menu(){
bool cur_menu_butt1_state;

  cur_menu_butt1_state = digitalRead(menuButtonPin1);
  //Serial.println(cur_menu_butt1_state); delay(100);
  if(cur_menu_butt1_state){   // button not pressed or released
    if(menu_button_time!=0){  // button released
      if(menu_button_time+menu_button_short>millis()){  // short press
        menu_short_press();
      }else{  // long press
        menu_long_press();
      }
      delay(100); // poors man debounce ;)
      menu_button_time=0;
    }
    return;
  }else{  // button pressed
    if(menu_button_time==0) menu_button_time=millis();
  }
}


void display_kills(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE); 
  display.drawRect(1, 1, 127, 31, WHITE);
  display.setCursor(6,7);
  display.print("Kills: ");
  display.println(mosq_kills);
  display.display();
//  Serial.print("audio:");
//  Serial.println(next_kill_audio);
  if(next_kill_audio<mosq_kills){
    if(prefs_audio) myDFPlayer.playFolder(3,random(mp3_3));
    next_kill_audio+=random(4,kill_sound_cnt);
  }
//  Serial.print(" naudio:");
//  Serial.println(next_kill_audio);
}


// Process mosquito kill
void kill_mosquito(){
uint32_t new_kill=millis();

  mosq_kills++;
  if(last_kill+kill_combo_threshold>new_kill){  // check if we have a combo?
    kombo_kill_count++;
    next_kill_audio++;  // to not double combo sound with standard sound
    Serial.print("Kombo kill: "); Serial.println(kombo_kill_count);
    if(prefs_audio) myDFPlayer.playFolder(4,kombo_kill_count);
  }else{
    kombo_kill_count=0;
  }
  display_kills();
  last_kill = new_kill;
}


// Check if the power button is pressed and debounce it
bool check_zap_button(){
bool cur_zap_butt_state;

  cur_zap_butt_state = !digitalRead(zapButtonPin);  // invert, because we have changed to PULLUP with transistor
  
  //Serial.println(cur_zap_butt_state);
  //delay(100);


// If the button state has changed - work on it's history
  if(cur_zap_butt_state != old_zap_butt_state && zap_butt_cnt==0){
    zap_butt_hist=(byte)cur_zap_butt_state;
    zap_butt_cnt=1;
    //Serial.println("Start debounce");
    return old_zap_butt_state;
  }

// We are debouncing...
  if(zap_butt_cnt>0){
    if(zap_butt_cnt>10){    // finish debounce
      if(zap_butt_hist<4){
        cur_zap_butt_state=0;
        zap_butt_cnt=0;
      }else if(zap_butt_hist>6){
        cur_zap_butt_state=1;
        zap_butt_cnt=0;
      }else{ // debounce failed
        zap_butt_cnt=1;
        zap_butt_hist=(byte)cur_zap_butt_state;
        return old_zap_butt_state;  // return previous value until manage to debounce 
      }
    }else{
      zap_butt_hist+=cur_zap_butt_state;
      zap_butt_cnt++;
      return old_zap_butt_state;  // return previous value until we debounce 
    }
  }

  if(cur_zap_butt_state>old_zap_butt_state){
    power_on();
  }else if(cur_zap_butt_state<old_zap_butt_state){
    power_off();
  }
  old_zap_butt_state=cur_zap_butt_state;
  
  return old_zap_butt_state;
}

// Check timeout for sentence and play it
void check_sentence(){
  if(millis()<next_sentence) return;
  next_sentence=millis()+random(min_delay_for_sentence,max_delay_for_sentence);
  if(prefs_audio) myDFPlayer.playFolder(5,random(mp3_5));
  Serial.println("Audi sentence played!");
  Serial.print(" Now is: "); Serial.println(millis());
  Serial.print(" Next sentence millis: "); Serial.println(next_sentence);
  Serial.print(" Next sentence seconds: "); Serial.println((int)((next_sentence-millis())/1000));
}

/*
** Initialize all stuff - arduino style
*/
void setup(void){

  delay(1200); // this is to make my nano boot from external power (https://forum.arduino.cc/t/nano-requires-manual-reset-when-not-connected-to-computer/495587/15)
  Serial.begin(115200);
  Serial.println("Fight!");
  randomSeed(analogRead(0));
  next_kill_audio = random(1,kill_sound_cnt);

  pinMode(menuButtonPin1, INPUT_PULLUP);
  
  mySoftwareSerial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.clearDisplay();
  display.setTextSize(1.6);
  display.setTextColor(WHITE); 
  display.drawRect(1, 1, 127, 31, WHITE);
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
  restore_score();
  myDFPlayer.volume(25);  //Set volume value. From 0 to 30
  if(prefs_audio) myDFPlayer.playFolder(6,1);

  pinMode(zapButtonPin, INPUT_PULLUP);
    
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV");
  ads.setGain(GAIN_ONE);
  ads.begin();

  next_sentence=millis()+random(min_delay_for_sentence,max_delay_for_sentence);  // set the next play sentence
}


void loop(void){
static byte count=0;
int16_t results;

  check_menu();
  check_sentence();

  if(!check_zap_button()) return; // if the zap button is not pressed - we no need to go further

  /* Be sure to update this value based on the IC and the gain settings! */
  float   multiplier = 3.0F;    /* ADS1015 @ +/- 6.144V gain (12-bit results) */
  //float multiplier = 0.1875F; /* ADS1115  @ +/- 6.144V gain (16-bit results) */

  results = ads.readADC_Differential_0_1();
/*  if(count>20){
    Serial.print("Differential: "); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)"); delay(100);
    count=0;
  }else count++;
*/  
  if(results>max_val){
    max_val=results;
    //Serial.print(".. setting new max"); Serial.println(max_val);
    //delay(100);
    if(max_val>total_max_val) total_max_val=max_val;
  }

  if(results<(max_val*0.5) && max_val>100){ // we have discharge
    Serial.println("Discharge!");
    if((last_kill+kill_grace_period)<millis()){  // if we pass grace perdiod (not a chain discharge)
      Serial.print("Kill! maxV:"); Serial.print(max_val); Serial.print(" res:"); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)");
      kill_mosquito();
    }
    max_val=results;  // despite if it's chain or not - we update current voltage value
  }
}
