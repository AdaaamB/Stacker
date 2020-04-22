// Stacker
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <MD_Parola.h>
#include "Font5x3.h"

// Use a button to transfer between transformations or just do it on a timer basis
#define USE_SWITCH_INPUT  1
#define SWITCH_PIN  9 // switch pin if enabled - active LOW

// We always wait a bit between updates of the display
static int DELAYTIME = 150;  // in milliseconds

// Number of times to repeat the transformation animations
#define REPEATS_PRESET  16

// Define the number of devices we have in the chain and the hardware interface
#define HARDWARE_TYPE MD_MAX72XX::ICSTATION_HW
#define MAX_DEVICES   4
#define CLK_PIN   13  // or SCK
#define DATA_PIN  11  // or MOSI
#define CS_PIN    10  // or SS

int dir = 1, prevX = 0, curX = 0, curY = 0, len = 3, prevLen = 3, isStart = 1, btnActive = 1, btnDelay = 300;
uint32_t btnLimit = 0;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void doStacker()
{
  static uint32_t lastTime = 0;
  if (millis() - lastTime >= DELAYTIME)
  {
    if (curX == 0 || curX == 7 + len - 1) dir = !dir;
    curX = (dir == 0) ? curX + 1 : curX - 1;
    mx.setPoint(curX + 1 - (len + 1) * !dir, curY, false);
    mx.setPoint(curX + 1 - (len + 1) * !dir, curY + 1, false);

    for (int i = 0; i < len; i++) {
      mx.setPoint(curX - i, curY, true);
      mx.setPoint(curX - i, curY + 1, true);
    }
    
    lastTime = millis();
  }
}

void hitButton() {
//  if (curX > 7) { //TODO ##########
//    len = abs(len + (7 - curX));
//    curX = curX - (curX - 7);
//    prevX = curX;
//  } else 
  
  if (prevX != curX) {
    len = len - abs(curX - prevX); //set len to be current len minus the difference between curX and prevX.
    for (int i = 0; i < abs(curX - prevX); i++) { //iterate i for the difference between curX and prevX to remove 1 or 2 points.
      Serial.print("x is "); Serial.println(curX);
      int j = (prevX < curX) ? curX - i : curX - len - i; //if overhang left, remove curX, else remove curX - len - i.
      mx.setPoint(j, curY, false); //remove overhang.
      mx.setPoint(j, curY + 1, false);
    }
    if (prevX < curX) curX = curX - abs(curX - prevX);
    prevX = curX; 
    }

    if (len < 1) reset(0);
    if (curY + 2 == 32) reset(1);

    prevLen = len;
    curY += 2;
    DELAYTIME -= 5;
    Serial.println("Hit button!");
    btnActive = 1;
}

void reset(int win) {
  Serial.println("reset called");

  if (win) {
    for (int i = 0; i < 15; i++) { flash_every_other(); }
    for (int i = 0; i < 4; i++) { 
      P.print("WINNER");
      delay(500);
      mx.clear();
      delay(500);
    }
    mx.clear();
  } else { //######## TODO fix it jumping back to look like a false loss ########
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < prevLen; j++) {
       mx.setPoint(curX + j, curY, true);
       mx.setPoint(curX + j, curY + 1, true);
      }
       delay(300);
      for (int j = 0; j < prevLen; j++) {
       mx.setPoint(curX + j, curY, false);
       mx.setPoint(curX + j, curY + 1, false);
      }
       delay(300);
    }
  }

  //######## reset vars TODO ########
  
  mx.clear();
}

void setup()
{
  mx.begin();
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  Serial.begin(57600);
  Serial.println("STACKER");
  P.begin();
  P.setFont(StackerFont);
  P.setTextAlignment(PA_CENTER);
  P.print("STACK");
  delay(1000);
  mx.clear();
}

void loop()
{
//  if (digitalRead(SWITCH_PIN) == HIGH) 
  doStacker();

  if (digitalRead(SWITCH_PIN) == LOW && btnActive && millis() > btnLimit) {
    if (isStart) {
      prevX = curX;
      isStart = 0;
    }
    btnActive = 0;
    btnLimit = millis() + btnDelay;
    hitButton();
  }
}

void flash_every_other(){
  for(int a = 0; a < 4; a++) {
    for (int i = 0; i < 8; i+=2){
     mx.setRow(a, i,170);
     mx.setRow(a, i + 1, 85);
    }
  }
  
  delay(50);
  mx.clear();
   
   for(int a = 0; a < 4; a++) {
    for (int i = 0; i < 8; i+=2){
     mx.setRow(a, i, 85);
     mx.setRow(a, i + 1 ,170);
    }
  }

  delay(50);
  mx.clear();
}
