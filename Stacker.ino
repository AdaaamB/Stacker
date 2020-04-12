// Program to exercise the MD_MAX72XX library
//
// Test the library transformation functions

#include <MD_MAX72xx.h>
#include <SPI.h>

// Use a button to transfer between transformations or just do it on a timer basis
#define USE_SWITCH_INPUT  1

#define SWITCH_PIN  9 // switch pin if enabled - active LOW

// We always wait a bit between updates of the display
#define  DELAYTIME  500  // in milliseconds

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

//static int curX[] = {0, 1, 2};
static int curX = 1;

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
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

void transformDemo(MD_MAX72XX::transformType_t tt, String dir, bool bNew, int curY)
{
  static uint32_t lastTime = 0;

  if (bNew)
  {
    for (int i = curX-1; i < curX+2; i++) { //TODO: 3 WILL CHANGE TO 2 THEN 1 BASED ON LENGTH
      mx.setPoint(i, curY, true);
      mx.setPoint(i, curY + 1, true);
    }
    Serial.println("Hit button!");
    lastTime = millis();
  }

  if (millis() - lastTime >= DELAYTIME)
  {
    if (dir.equals("left")) {
      Serial.println("Going left");
      mx.setPoint(curX - 1, curY, false);
      mx.setPoint(curX - 1, curY + 1, false);
      
      for (int i = curX-1; i < curX+2; i++) { //TODO: 3 WILL CHANGE TO 2 THEN 1 BASED ON LENGTH
        mx.setPoint(i+1, curY, true);
        mx.setPoint(i+1, curY + 1, true);
      }
      curX = curX + 1;
    } else {
      Serial.println("Going right");
      mx.setPoint(curX + 1, curY, false);
      mx.setPoint(curX + 1, curY + 1, false);
      
      for (int i = curX+1; i > curX-2; i--) { //TODO: 3 WILL CHANGE TO 2 THEN 1 BASED ON LENGTH
        mx.setPoint(i-1, curY, true);
        mx.setPoint(i-1, curY + 1, true);
      }
      curX = curX - 1;
    }
    lastTime = millis();
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
}

void loop()
{
  static int8_t tState = 0;
  static bool bNew = true;
  static int curY = -2;

  if (bNew)
  {
    curY = curY + 2;
  }

  if (mx.getPoint(7,curY)) {
    tState = 0;
    Serial.println("Hit left side");
  }
  if (mx.getPoint(0,curY)) {
    tState = 1;
    Serial.println("Hit right side");
  }

  switch (tState)
  {
    case 0: transformDemo(MD_MAX72XX::TSU, "right", bNew, curY);  break;
    case 1: transformDemo(MD_MAX72XX::TSD, "left", bNew, curY);  break;
    //case 7: transformDemo(MD_MAX72XX::TINV, bNew);  break; //TODO THIS IS A GREAT STATE FOR LOSING
    default:  tState = 0; // just in case
  }

  bNew = changeState();
}
