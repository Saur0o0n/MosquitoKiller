#include <Adafruit_ADS1X15.h>
#include <Adafruit_SSD1306.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

SoftwareSerial mySoftwareSerial(10, 9); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);
const byte mp3_1 = 9; // start
const byte mp3_2 = 9; // stop
const byte mp3_3 = 10; // victory
const byte mp3_4 = 0; // kill

// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

const byte buttonPin = 12;
bool old_butt_state = 0;
uint8_t butt_hist = 0, butt_cnt = 0;
uint16_t mosq_kills = 0;

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


void setup(void)
{
  Serial.begin(115200);
  Serial.println("Hello!");
  mySoftwareSerial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC,0x3C);
  display.clearDisplay();
  display.setTextSize(1.6);
  display.setTextColor(WHITE); 
  display.drawRect(1, 1, 126, 30, WHITE);
  display.setCursor(7,10);
  display.println("* Mosquito Killer *");
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
  
  Serial.println("Getting differential reading from AIN0 (P) and AIN1 (N)");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  pinMode(buttonPin, INPUT);
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();
}

int16_t max_val=10;

void power_off(){
  Serial.println("Power OFF :(");
  max_val=10;
  myDFPlayer.playFolder(2,random(mp3_2));
}

void power_on(){
  Serial.println("Power ON!");
  myDFPlayer.playFolder(1,random(mp3_1));
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
  myDFPlayer.playFolder(3,random(mp3_3));
}

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

void loop(void)
{
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

  if(results<(max_val*0.8) && max_val>100){
    Serial.print("Kill! maxV:"); Serial.print(max_val); Serial.print(" res:"); Serial.print(results); Serial.print("("); Serial.print(results * multiplier); Serial.println("mV)");
    max_val=results;
    mosq_kills++;
    display_kills();
    delay(500);
  }
}
