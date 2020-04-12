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

// SPI hardware interface
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary pins
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


bool changeState(void)
{
  bool b = false;

#if USE_SWITCH_INPUT

  static int8_t	lastStatus = HIGH;
  int8_t	status = digitalRead(SWITCH_PIN);

  b = (lastStatus == HIGH) && (status == LOW);
  lastStatus = status;
#else
//TODO LEFT THIS CODE IN AS IT'S ON A TIMER, WHICH MAY BE USEFUL FOR SCREENSAVER EFFECTS
  static uint32_t	lastTime = 0;
  static uint8_t	repeatCount = 0;

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

void transformDemo(MD_MAX72XX::transformType_t tt, bool bNew, int row)
{
  static uint32_t lastTime = 0;

  if (bNew)
  {
    //mx.clear();

    for (uint8_t i=0; i<MAX_DEVICES; i++)
    //mx.setChar(((i+1)*COL_SIZE)-1, 'o'+i);
    mx.setPoint(0, row, true);
    mx.setPoint(1, row, true);
    mx.setPoint(2, row, true);
    mx.setPoint(0, row + 1, true);
    mx.setPoint(1, row + 1, true);
    mx.setPoint(2, row + 1, true);
    lastTime = millis();
  }

  if (millis() - lastTime >= DELAYTIME)
  {
    mx.transform(0, MAX_DEVICES-1, tt);
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
  static int row = -2;

  if (bNew)
  {
    //tState = (tState+1) % 2;
    //Serial.print("State: "); Serial.println(tState);
    row = row + 2;
  }

  if (mx.getPoint(7,row)) {
    tState = 0;
    Serial.println("Hit right side");
  }
  if (mx.getPoint(0,row)) {
    tState = 1;
    Serial.println("Hit left side");
  }

  switch (tState)
  {
    case 0: transformDemo(MD_MAX72XX::TSU, bNew, row);  break;
    case 1: transformDemo(MD_MAX72XX::TSD, bNew, row);  break;
    //case 7: transformDemo(MD_MAX72XX::TINV, bNew);  break; //TODO THIS IS A GREAT STATE FOR LOSING
    default:  tState = 0; // just in case
  }

  bNew = changeState();
}
