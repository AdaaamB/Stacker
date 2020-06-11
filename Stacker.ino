// Stacker
#include <MD_MAX72xx.h>
#include <MD_Parola.h>
#include <SPI.h>
#include <OneButton.h>
#include "Font5x3.h"

// Use a button to transfer between transformations or just do it on a timer basis
#define USE_SWITCH_INPUT  1
#define SWITCH_PIN  9 // switch pin if enabled - active LOW
OneButton button(9, true);

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

int dir = 1, prevX = 0, curX = 0, curY = 0, len = 3, prevLen = 3, difficulty = 0, inProgress = 0, isStart = 1, btnActive = 1, btnDelay = 300;
uint32_t btnLimit = 0;
boolean btnLastState = HIGH;

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void doStacker()
{
  static uint32_t lastTime = 0;
  if (millis() - lastTime >= DELAYTIME)
  {
    if (curX == 0 || curX == 7 + len - 1) dir = !dir; //if almost off the board, reverse the direction.
    curX = (dir == 0) ? curX + 1 : curX - 1; //if dir is 0, move right, else move left.
    int removePoint = (dir == 0) ? curX - len : curX + 1; //set removePoint to remove trailing point depending on direction.
    mx.setPoint(removePoint, curY, false); //remove the trail.
    mx.setPoint(removePoint, curY + 1, false);

    for (int i = 0; i < len; i++) { //repeat for len to make the trail.
      mx.setPoint(curX - i, curY, true); //turn the point on.
      mx.setPoint(curX - i, curY + 1, true);
    }
    
    lastTime = millis();
  }
}

void hitButton() {
  if (isStart) { //if it's the first button press of the game.
    if (curX > 7) { //if curX is off the left side of the screen.
      len = abs(len - (curX - 7)); //set len to be the current len minus the difference between curX and 7.
      curX = curX - (curX - 7); //set curX to be curX minus the difference between curX and 7.
    } else if (curX < 2) { //if curX is less than 2, at least 1 point is off the right side of the screen.
      len = (curX == 0) ? 1 : 2; //if curX is 0, set length to be 1, else set it to be 2.
    }
    prevX = curX; //set prevX so no more calculations are done.
    isStart = 0; //no longer the first button press of the game.
  }
  
  if (prevX != curX && prevX - prevLen != curX - len) { //if prevX and curX are different, or if prevX - prevLen are different to curX - len -- this is for when we do len--.
    len = (prevX < curX) ? len - abs(curX - prevX) : prevLen - abs(curX - prevX); //set len to be current len or prevLen depending on overhang direction minus the difference between curX and prevX.
    for (int i = 0; i < abs(curX - prevX); i++) { //iterate i for the difference between curX and prevX to remove 1 or 2 points.
      int j = (prevX < curX) ? curX - i : curX - len - i; //if overhang left, remove curX, else remove curX - len - i.
      mx.setPoint(j, curY, false); //remove overhang.
      mx.setPoint(j, curY + 1, false);
    }
    if (len > 0) doFall((prevX < curX) ? curX : curX - len, curY, abs(curX - prevX)); //animate falling with either curX or curX - len depending on overhang direction, curY value and length of points fallen.
    if (len < 1) { reset(0); return; } //invoke reset with win = 0. return to stop further code in this method.
    if (prevX < curX) curX = curX - abs(curX - prevX); //this does something about keeping X correct as the tower gets smaller ######## to rewrite
    
  }

  prevX = curX; //set prevX to curX for next comparison.
  if (curY + 2 == 32) { reset(1); return; } //invoke reset with win = 1. return to stop further code in this method.
  
  prevLen = len; //set prevLen to len for next comparison.
  if (curY == 8 && len == 3) len--; //if len is still 3 after curY is 5th button press, remove one from len.
  if (curY == 18 && len == 2) len--; //if len is still 2 after curY is 10th button press, remove one from len.
  
  curY += 2;
  DELAYTIME -= (difficulty) ? 8 : 5;
  btnActive = 1;
}

void doFall(int x, int y, int l) { //animate falling, x coord, y coord and length.
  bool removeIt = false;
  if (l == 2) x--; //remove 1 from x if l is 2, as for some reason it has the wrong value.
  for (int i = y; i > -1; i--) { //repeat until y is off the board.
    removeIt = false; //reset removeIt to false to determine whether to perform the setPoint false.
    for (int j = x; j < x + l; j++) { //repeat for length of points to fall, will be either 1 or 2.
      if (!mx.getPoint(j, i)) { //if the point isn't already lit.
        mx.setPoint(j, i, true); //light the point.
        removeIt = true; //set removeIt to true to turn off the point later.
      }
    }
    delay(50);
    for (int j = x; j < x + l; j++) if (removeIt) mx.setPoint(j, i, false); //repeat for length of points to fall, turn off the point if it was lit in this method (removeIt val).
  }
}

void reset(int win) {
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
    for (int i = 0; i < 4; i++) { //repeat 4 times to point flash on and off.
      for (int j = 0; j < prevLen; j++) { //repeat for prevLen to flash 1, 2 or 3 points.
       mx.setPoint(curX - j, curY, true); //turn the point on.
       mx.setPoint(curX - j, curY + 1, true);
      }
       delay(300);
      for (int j = 0; j < prevLen; j++) { //repeat for prevLen to flash 1, 2 or 3 points.
       mx.setPoint(curX - j, curY, false); //turn the point off.
       mx.setPoint(curX - j, curY + 1, false);
      }
       delay(300);
    }
  }

  dir = 1; prevX = 0; curX = 0; curY = 0; len = 3; prevLen = 3; inProgress = 0; isStart = 1; btnActive = 1; btnLimit = 0; DELAYTIME = 150; //reset variables.
  mx.clear();
}

void setup()
{
  button.attachDoubleClick(doubleClick); //link the function to be called on a doubleclick event.
  button.attachClick(singleClick); //link the function to be called on a singleclick event.
  
  mx.begin();
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  Serial.begin(57600);
  Serial.println("STACKER");
  P.begin();
  P.setFont(StackerFont);
  P.setTextAlignment(PA_CENTER);
  P.print("STACKR");
  delay(1000);
  mx.clear();
  scrollText("CHOOSE MODE");
}

void scrollText(char curMessage[75]) {
  P.displayClear();
  P.displayScroll(curMessage, PA_CENTER, PA_SCROLL_LEFT, 30);
}

void loop()
{  
  if (inProgress) {
    if (digitalRead(SWITCH_PIN) == HIGH) doStacker();

    if (digitalRead(SWITCH_PIN) == LOW && btnActive && millis() > btnLimit) {
      btnActive = 0;
      btnLimit = millis() + btnDelay;
      hitButton();
    }
  } else {
    if (isStart) {
      if (P.displayAnimate()) {
        mx.clear();
        P.setFont(StackerFont);
        P.print("EASY");
        isStart = 0;
      }
    }
    button.tick(); //check the status of the button for OneButton
  }
}

void singleClick() {
  P.print((difficulty) ? "EASY" : "HARD");
  difficulty = !difficulty;
}

void doubleClick() {
  inProgress = 1;
  isStart = 1;
  mx.clear();
  btnLimit = millis() + btnDelay;
  delay(500);
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
