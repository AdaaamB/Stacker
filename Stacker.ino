// Program to exercise the MD_MAX72XX library
//
// Test the library transformation functions

#include <MD_MAX72xx.h>
#include <SPI.h>
#include <MD_Parola.h>
#include "Font5x3.h"

// Use a button to transfer between transformations or just do it on a timer basis
#define USE_SWITCH_INPUT  1

#define SWITCH_PIN  9 // switch pin if enabled - active LOW

// We always wait a bit between updates of the display
static int DELAYTIME = 300;  // in milliseconds
static int buttonDelay = 300;

// Number of times to repeat the transformation animations
#define REPEATS_PRESET  16

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES   4
#define WRAPAROUND_MODE MD_MAX72XX::OFF

#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

uint8_t scrollSpeed = 25;    // default frame delay value
textEffect_t scrollEffect = PA_SCROLL_RIGHT;
textPosition_t scrollAlign = PA_CENTER;
uint16_t scrollPause = 1000; // in milliseconds

static int prevX = 0;
static int curX = 0;
static int curY = -2;
static int len = 3;
static bool hit = false;
static bool button = false;
static uint32_t buttonTime = 0;
static uint32_t buttonLimit = 0;
static bool start = true;

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


bool changeState(void)
{
  bool b = false;

#if USE_SWITCH_INPUT

  static int8_t  lastStatus = HIGH;
  int8_t  status = digitalRead(SWITCH_PIN);

  b = (lastStatus == HIGH) && (status == LOW);
  lastStatus = status;
#else
//TODO LEFT THIS CODE IN AS IT'S ON A TIMER, WHICH MAY BE USEFUL FOR SCREENSAVER EFFECTS
  static uint32_t lastTime = 0;
  static uint8_t  repeatCount = 0;

  if (repeatCount == 0)
    repeatCount = REPEATS_PRESET;

  if (millis()-lastTime >= DELAYTIME)
  {
    lastTime = millis();
    b = (--repeatCount == 0);
  }
#endif

  return(b);
}

void transformDemo(MD_MAX72XX::transformType_t tt, String dir, bool bNew)
{
  static uint32_t lastTime = 0;

  if (bNew)
  {
    Serial.print("prevX is "); Serial.print(prevX); Serial.print(" curX is "); Serial.println(curX);
    if (prevX != curX) {
      len = len - abs(curX - prevX);
      if (prevX < curX) { //overhang on the left
        mx.setPoint(curX + len, curY, false);
        mx.setPoint(curX + len, curY + 1, false);
        mx.setPoint(curX + len + 1, curY, false);
        mx.setPoint(curX + len + 1, curY + 1, false); //TODO - ANIMATE THIS FOR SEXINESS and there must be a better way
        prevX = curX;
      } else { //overhang on the right
        mx.setPoint(curX, curY, false);
        mx.setPoint(curX, curY + 1, false);
        if (prevX - curX == 2) {
          mx.setPoint(curX + 1, curY, false);
          mx.setPoint(curX + 1, curY + 1, false);
          curX++;
        }
        curX++;
        prevX = curX;
      }
    }
    Serial.print("len is "); Serial.println(len);

    if (len < 1) reset(0); //TODO - ANIMATE/FLASH THE LOST PIXELS?
    if (curY + 2 == 32) reset(1);
 
    curY = curY + 2;
    DELAYTIME = DELAYTIME - 15;
    Serial.print("curY is "); Serial.println(curY);
    Serial.print("delay time "); Serial.println(DELAYTIME);
    
    for (int i = curX; i < curX + len; i++) {
      mx.setPoint(i, curY, true);
      mx.setPoint(i, curY + 1, true);
    }
    Serial.println("Hit button!");
    lastTime = millis();
  }

  if (millis() - lastTime >= DELAYTIME)
  {
    if (dir.equals("left")) {
      curX++;
      //Serial.println("Going left");
      mx.setPoint(curX - 1, curY, false);
      mx.setPoint(curX - 1, curY + 1, false);
      
      for (int i = curX; i < curX + len; i++) {
        mx.setPoint(i, curY, true);
        mx.setPoint(i, curY + 1, true);
      }
    } else {
      curX--;
      //Serial.println("Going right");
      mx.setPoint(curX + len, curY, false);
      mx.setPoint(curX + len, curY + 1, false);
      
      for (int i = curX; i < curX + len; i++) {
        mx.setPoint(i, curY, true);
        mx.setPoint(i, curY + 1, true);
      }
    }
    lastTime = millis();
    hit = false;
    button = true;
  }
}

void reset(int win) {
  DELAYTIME = 300;
  prevX = 0;
  curX = 0;
  curY = -2;
  len = 3;
  hit = false;
  button = false;
  buttonTime = 0;
  buttonLimit = 0;
  start = true;
  mx.clear();

  if (win) {
    for (int i = 0; i < 15; i++) { flash_every_other(); }
    for (int i = 0; i < 4; i++) { 
      P.print("WINNER");
      delay(500);
      mx.clear();
      delay(500);
    }
    mx.clear();
  } else {
    P.print("LOSER");
    delay(1500);
    mx.clear();
  }
}

void setup()
{
  mx.begin();
  // use wraparound mode
  mx.control(MD_MAX72XX::WRAPAROUND, WRAPAROUND_MODE);

#if USE_SWITCH_INPUT
  pinMode(SWITCH_PIN, INPUT_PULLUP);
#endif

  Serial.begin(57600);
  Serial.println("[Transform Test]");
  P.begin();
  P.setFont(_Fixed_5x3);
  P.setTextAlignment(PA_CENTER);
  P.print("STACK");
  delay(1000);
  mx.clear();
}

void loop()
{
  static int8_t tState = 1;
  static bool bNew = true;

  if (mx.getPoint(7,curY) and hit == false) {
    tState = 0;
    hit = true;
    //Serial.println("Hit left side");
  }
  if (mx.getPoint(0,curY) and hit == false) {
    tState = 1;
    hit = true;
    //Serial.println("Hit right side");
  }

  switch (tState)
  {
    case 0: transformDemo(MD_MAX72XX::TSU, "right", bNew);  break;
    case 1: transformDemo(MD_MAX72XX::TSD, "left", bNew);  break;
    //case 7: transformDemo(MD_MAX72XX::TINV, bNew);  break; //TODO THIS IS A GREAT STATE FOR LOSING
    default:  tState = 0; // just in case
  }

  if (digitalRead(SWITCH_PIN) == LOW && button && millis() > buttonLimit) {
      if (start) {
        prevX = curX;
        start = 0;
      }
      bNew = true; //TODO - THIS DOESN'T WORK
      button = false;
      buttonTime = millis();
      buttonLimit = buttonTime + buttonDelay;
  } else {
    bNew = false;
  }

  //bNew = changeState();
}

void flash_every_other(){
  for(int a=0; a<4; a++) {
    for (int i=0; i<8; i+=2){
     mx.setRow(a,i,170);
     mx.setRow(a,i+1,85);
    }
  }
  
  delay(50);
  mx.clear();
   
   for(int a=0; a<4; a++) {
    for (int i=0; i<8; i+=2){
     mx.setRow(a,i,85);
     mx.setRow(a,i+1,170);
    }
  }

  delay(50);
  mx.clear();
}
