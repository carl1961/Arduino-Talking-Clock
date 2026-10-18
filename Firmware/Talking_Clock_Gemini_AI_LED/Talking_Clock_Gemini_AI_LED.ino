#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_VS1053.h>
#include <TM1637Display.h>
#include <Adafruit_NeoPixel.h>

// --- VERIFIED HW-261 PIN DEFINITIONS ---
#define VS1053_RST    8     
#define VS1053_CS     6     
#define VS1053_DCS    7     
#define VS1053_DREQ   2     
#define CARDCS        9     

// --- BUTTON PINS ---
#define SAY_TIME_BTN  5     
#define MODE_BTN      4     
#define INC_BTN       10    

// --- TM1637 DISPLAY PINS ---
#define CLK_PIN       A0
#define DIO_PIN       A1

// --- NEOPIXEL / LED STRIP CONFIGURATION ---
#define LED_PIN       A2    
#define NUM_PIXELS    12    

Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(VS1053_RST, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

RTC_DS3231 rtc;
TM1637Display display(CLK_PIN, DIO_PIN);
Adafruit_NeoPixel strip(NUM_PIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Clock States
enum ClockState { NORMAL, SET_HOUR, SET_MINUTE };
ClockState currentState = NORMAL;

int tempHour = 12;
int tempMinute = 0;
int lastMinuteChecked = -1;
int lastRoosterDay = -1; // Tracks the last day the rooster crowed
unsigned long lastButtonPress = 0;
const unsigned long debounceDelay = 300; 

void setup() {
  Serial.begin(9600);
  Serial.println(F("Initializing Talking Clock (24-Hour Setting Mode)..."));

  pinMode(SAY_TIME_BTN, INPUT_PULLUP);
  pinMode(MODE_BTN, INPUT_PULLUP);
  pinMode(INC_BTN, INPUT_PULLUP);

  display.setBrightness(0x0f);

  // Initialize NeoPixels
  strip.begin();
  strip.show(); 
  setGnomeIdleColor(); 

  if (!rtc.begin()) {
    Serial.println(F("Couldn't find RTC!"));
    while (1);
  }

  if (!musicPlayer.begin()) {
    Serial.println(F("Couldn't find VS1053!"));
    while (1);
  }

  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD Card failed!"));
    while (1);
  }

  musicPlayer.setVolume(20, 20);
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);
  
  Serial.println(F("Ready!"));
}

void loop() {
  DateTime now = rtc.now();
  int currentMinute = now.minute();
  int currentHour24 = now.hour(); // Get true 24h hour from RTC

  // --- BUTTON & MODE LOGIC ---
  if (digitalRead(MODE_BTN) == LOW && (millis() - lastButtonPress > debounceDelay)) {
    lastButtonPress = millis();
    if (currentState == NORMAL) {
      currentState = SET_HOUR;
      tempHour = currentHour24; // Start editing from current 24h hour
      Serial.println(F("Mode: Setting Hour (24h)"));
    } 
    else if (currentState == SET_HOUR) {
      currentState = SET_MINUTE;
      tempMinute = now.minute();
      Serial.println(F("Mode: Setting Minute"));
    } 
    else if (currentState == SET_MINUTE) {
      currentState = NORMAL;
      
      // Directly save the 24-hour time to the RTC
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), tempHour, tempMinute, 0));
      Serial.println(F("24-Hour Time Saved to RTC!"));
    }
  }

  if (digitalRead(INC_BTN) == LOW && (millis() - lastButtonPress > debounceDelay)) {
    lastButtonPress = millis();
    if (currentState == SET_HOUR) {
      tempHour++;
      if (tempHour > 23) {
        tempHour = 0; // Wraps from 23 back to midnight (0)
      }
    } else if (currentState == SET_MINUTE) {
      tempMinute++;
      if (tempMinute > 59) tempMinute = 0;
    }
  }

  // --- NORMAL VS SETTING OPERATIONS ---
  if (currentState == NORMAL) {
    // Show 12-hour format on display for normal viewing (converting 24h to 12h for the display numbers)
    int displayHour = currentHour24 % 12;
    if (displayHour == 0) displayHour = 12;
    display.showNumberDecEx(displayHour * 100 + now.minute(), 0b01000000, true);

    // --- 6:01 AM STRICT MORNING ROOSTER ALARM CHECK ---
    if (currentHour24 == 6 && currentMinute == 0) {
      if (now.day() != lastRoosterDay) {
        lastRoosterDay = now.day(); 
        
        Serial.println(F("Good morning! Playing rooster alarm..."));
        musicPlayer.playFullFile("/rooster.mp3");
       // announceTimeWithAnimation(currentHour24, currentMinute);
      }
    }

    // 15-Minute Automatic Check
    if ((currentMinute == 0 || currentMinute == 15 || currentMinute == 30 || currentMinute == 45) 
        && currentMinute != lastMinuteChecked) {
      lastMinuteChecked = currentMinute;
      announceTimeWithAnimation(currentHour24, currentMinute);
    }
    
    if (currentMinute != 0 && currentMinute != 15 && currentMinute != 30 && currentMinute != 45) {
      lastMinuteChecked = -1;
    }

    // Manual Button Check
    if (digitalRead(SAY_TIME_BTN) == LOW) {
      if (millis() - lastButtonPress > debounceDelay) {
        lastButtonPress = millis();
        announceTimeWithAnimation(currentHour24, currentMinute);
      }
    }
  } 
  else if (currentState == SET_HOUR) {
    // Display the 24-hour hour value while setting (e.g., 1400 for 2 PM)
    display.showNumberDecEx(tempHour * 100 + 0, 0b01000000, true);
  } 
  else if (currentState == SET_MINUTE) {
    display.showNumberDecEx(tempHour * 100 + tempMinute, 0b01000000, true);
  }
  
  delay(100);
}

// --- AUDIO ANNOUNCEMENT WITH LED ANIMATION ---
void announceTimeWithAnimation(int hour24, int minute) {
  pulseGNOME();

  int displayHour = hour24 % 12;
  if (displayHour == 0) displayHour = 12;

  char audioFile[15];

  // Play Hour File
  sprintf(audioFile, "/h%d.mp3", displayHour);
  musicPlayer.playFullFile(audioFile);

  // Play Minute Files
  if (minute <= 20) {
    sprintf(audioFile, "/m%d.mp3", minute);
    musicPlayer.playFullFile(audioFile);
  } 
  else {
    int tens = (minute / 10) * 10;
    int units = minute % 10;

    sprintf(audioFile, "/m%d.mp3", tens);
    musicPlayer.playFullFile(audioFile);

    if (units > 0) {
      sprintf(audioFile, "/u%d.mp3", units);
      musicPlayer.playFullFile(audioFile);
    }
  }

  setGnomeIdleColor();
}

// --- LED ANIMATION FUNCTIONS ---
void setGnomeIdleColor() {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(75, 0, 130)); // Indigo / Purple
  }
  strip.show();
}

void pulseGNOME() {
  for (int b = 50; b <= 255; b += 50) {
    for (int i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, strip.Color(0, b, b)); // Bright Cyan pulse
    }
    strip.show();
    delay(50);
  }
}