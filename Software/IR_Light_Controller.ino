/*
  LED light strip controller using Media Center Remote (RC6 protocol)
  http://hazarkarabay.com.tr
*/

/* Defines */
#define TOP 0x0FFF // 12-bit resolution, 3906 Hz PWM

// Microsoft Media Center Remote teletext button codes
#define BUTTON_TOGGLE 0x800F045B // Red
#define BUTTON_BRI_DEC 0x800F045C // Green
#define BUTTON_BRI_INC 0x800F045D // Yellow
#define BUTTON_BREATHE 0x800F045E // Blue
// Buttons have two states, base address and +0x8000

#define BRI_STEP 5
#define CMD_TOGGLE_INTERVAL 200 // minimum time between light toggle
#define BRI_SET_INTERVAL 10 // fade in/out step time
#define LDR_THRESHOLD 900 // if LDR reading is greater than this, it is dark.

//#define DEBUG

/* Variables */
byte brightness = 0; // Current brightness, 0-100
byte targetBrightness = 0; // Wanted brightness, also 0-100. Used for fade effect while changing.
float luminance = 0; // 0-1
unsigned int pwmValue = 0; // 0-4096 (12-bit)
unsigned long lastCommand = 0;
unsigned long lastBrightnessChange = 0;
unsigned long lastLDRChange = 0;
unsigned int LDRValue = 0;
bool LDRStatus = false; // true=dark, false=lit

/* Includes */
#include "Functions.h"
#include <IRLibRecv.h>
#include <IRLibDecodeBase.h>
#include <IRLib_P04_RC6.h>
#include <IRLibCombo.h>

/* Library init */
IRrecv myReceiver(2);  // pin number for the IR receiver
IRdecode myDecoder;

void setup() {
  // Timer1 PWM Setup - Fixed on PB1, Arduino pin 9
  // Register init
  TCCR1A = 0;  // Timer/Counter1 Control Register A
  TCCR1B = 0;  // Timer/Counter1 Control Register B
  TIMSK1 = 0;   // Timer/Counter1 Interrupt Mask Register
  TIFR1 = 0;   // Timer/Counter1 Interrupt Flag Register
  ICR1 = TOP;
  OCR1A = 0; // On the contrary of popular belief, value of 0 *will* output some PWM.
  OCR1B = 0; //

  // Set clock prescale to 1 for maximum PWM frequency
  TCCR1B |= _BV(CS10);

  // Set to Timer/Counter1 to Waveform Generation Mode 14: Fast PWM with TOP set by ICR1
  TCCR1A |= _BV(WGM11);
  TCCR1B |= _BV(WGM13) | _BV(WGM12);

  // Set PB1 output
  pinMode(9, OUTPUT);

  myReceiver.enableIRIn(); // Start the receiver

#ifdef DEBUG
  Serial.begin(115200);
  Serial.println(F("Can you hear me?"));
#endif
}

void loop() {
  // IR receive and decode stuff
  if (myReceiver.getResults()) {
    if (myDecoder.decode()) {
      switch (myDecoder.value) {
        case BUTTON_TOGGLE:
        case BUTTON_TOGGLE+0x8000:
          // Throttle IR command processing
          if (millis() > lastCommand + CMD_TOGGLE_INTERVAL) {
            DEBUG_PRINT(F("TOGGLE"));
            targetBrightness = (targetBrightness) ? 0 : 100;
          }
          break;
        case BUTTON_BRI_DEC:
        case BUTTON_BRI_DEC+0x8000:
          DEBUG_PRINT(F("DEC"));
          if (targetBrightness > BRI_STEP)
            targetBrightness -= BRI_STEP;
          else targetBrightness = 0;
          break;
        case BUTTON_BRI_INC:
        case BUTTON_BRI_INC+0x8000:
          DEBUG_PRINT(F("INC"));
          if (targetBrightness + BRI_STEP > 100)
            targetBrightness = 100;
          else targetBrightness += BRI_STEP;
          break;
        case BUTTON_BREATHE:
        case BUTTON_BREATHE+0x8000:
          // TODO: Insert crazy light effect here.
          DEBUG_PRINT(F("*heavy breating*"));
          break;
      }
      lastCommand = millis();
    }

    myReceiver.enableIRIn();
  }

  // Application logic
  LDRValue = analogRead(A0);

  if (LDRValue > LDR_THRESHOLD && !LDRStatus) {
    // Currently dark but previous status is lit
    lastLDRChange = millis();
    LDRStatus = true;

    DEBUG_PRINT2(F("LDR Status Changed: "), LDRStatus);
  } else if (LDRValue < LDR_THRESHOLD && LDRStatus) {
    // Currently lit but previous status is dark
    lastLDRChange = millis();
    LDRStatus = false;

    DEBUG_PRINT2(F("LDR Status Changed: "), LDRStatus);
  }

  // Making a decision at first power on immediately
  if (lastLDRChange == 0) {
    targetBrightness = (LDRStatus) ? 100 : 0;
    DEBUG_PRINT2(F("Power-on decision was made. Brightness set to "), targetBrightness);
    delay(1);
    lastLDRChange = millis();
  }


  // Fade in/out effect while changing brightness
  if (millis() - lastBrightnessChange > BRI_SET_INTERVAL) {
    lastBrightnessChange = millis();
    if (targetBrightness > brightness) {
      brightness++;
    } else if (targetBrightness < brightness) {
      brightness--;
    }

    setBrightness(brightness);
    //DEBUG_PRINT(brightness);
  }
}

/* Helper functions */

inline void setPWM(unsigned int val) {
  if (val == 0) {
    // Disconnecting timer to taking pin under our control.
    TCCR1A &= ~_BV(COM1A1);
    digitalWrite(2, LOW);
  } else {
    // If needed: Connect pin to timer
    if (OCR1A == 0)
      TCCR1A |= _BV(COM1A1);
  }
  OCR1A = constrain(val, 0, TOP);
}

inline unsigned int calculatePWM(byte bri) {
  bri = constrain(bri, 0, 100);

  // Perceived lightness for humans - using psychometric lightness formula, CIE 1931
  // http://photonstophotos.net/GeneralTopics/Exposure/Psychometric_Lightness_and_Gamma.htm
  if (bri > 8) {
    luminance =  pow(((bri + 16.0) / 116), 3);
  } else {
    luminance = (bri / 903.3);
  }

  return int(luminance * TOP); // Adjust to required resolution
}

inline void setBrightness(byte val) {
  setPWM(calculatePWM(val));
}

