
// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Arduino.h>

#include <ezButton.h>  // the library to use for SW pin
#include "Encoder.h"
#include <ESP32Encoder.h>

#include <Adafruit_BNO055.h>

#define CLK_PIN 5  // ESP32 pin GPIO25 connected to the rotary encoder's CLK pin
#define DT_PIN  18 // ESP32 pin GPIO26 connected to the rotary encoder's DT pin
#define SW_PIN  19 // ESP32 pin GPIO27 connected to the rotary encoder's SW pin

//Encoder encoder(DT_PIN, CLK_PIN);
ESP32Encoder encoder;


ezButton button(SW_PIN);

Adafruit_BNO055 bno;

#include <FastLED.h>


#include "LEDMatrix.h"
#include "LEDText.h"
#include "FontRobotron.h"
#include "FontMatrise.h"


#define MATRIX_WIDTH   69
#define MATRIX_HEIGHT  18
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> ledsm;

cLEDText ScrollingMsg;

const unsigned char PlasmaTxt[] = { EFFECT_BACKGND_LEAVE EFFECT_RGB "\xff\xff\xff" "         F-PLASMA " EFFECT_DELAY_FRAMES "\x01\x2c" "         "
                                    EFFECT_BACKGND_DIMMING "\x40" EFFECT_RGB "\xff\xff\xff" "         F-PLASMA " EFFECT_DELAY_FRAMES "\x01\x2c" "         "
                                    EFFECT_BACKGND_LEAVE EFFECT_COLR_DIMMING "\x10" "         F-PLASMA " EFFECT_DELAY_FRAMES "\x01\x2c" "         "
                                    EFFECT_BACKGND_ERASE EFFECT_COLR_EMPTY "     F-PLASMA " EFFECT_DELAY_FRAMES "\x01\x2c" "     "
                                    EFFECT_BACKGND_ERASE EFFECT_COLR_DIMMING "\x40" "     F-PLASMA " EFFECT_DELAY_FRAMES "\x01\x2c" "     " };


const unsigned char TxtRainbowL[] = { EFFECT_SCROLL_LEFT "     " EFFECT_HSV "\x00\xff\xff" "H" EFFECT_HSV "\x20\xff\xff" "+" EFFECT_HSV "\x40\xff\xff" "V" EFFECT_HSV "\x60\xff\xff" "=" EFFECT_HSV "\x80\xff\xff" "<3"};



// Here's how to control the LEDs from any two pins:
#define DATAPIN    13

const uint8_t kMatrixWidth = 69;
const uint8_t kMatrixHeight = 18;
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
#define LAST_VISIBLE_LED 1000

CRGB leds[ NUM_LEDS ];

uint16_t XYTable[NUM_LEDS];

uint16_t XY( uint8_t x, uint8_t y)
{
  // any out of bounds address maps to the first hidden pixel
  if ( (x >= kMatrixWidth) || (y >= kMatrixHeight) ) {
    return (LAST_VISIBLE_LED + 1);
  }

  uint16_t i = (y * kMatrixWidth) + x;
  uint16_t j = XYTable[i];
  return j;
}


void setup() 
{  

  // initialize the matrix values
  for(int i = 0; i < NUM_LEDS; ++i) {
    XYTable[i] = (i % 2 == 0) ? i / 2 : LAST_VISIBLE_LED;
  }
  
  //encoder.attachFullQuad ( DT_PIN, CLK_PIN );
  encoder.attachSingleEdge ( DT_PIN, CLK_PIN );
  encoder.setCount ( 0 );

  
  Serial.begin(115200);

  Serial.println("Initializing IMU...");
  if(!bno.begin()) {
      Serial.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  } else {
    Serial.println("BNO055 connected!");
    delay(1000); // FIXME: do we need that?
    bno.setExtCrystalUse(true);
  }
  

  button.setDebounceTime(50);  // set debounce time to 50 milliseconds

  FastLED.addLeds<WS2812B, DATAPIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
  FastLED.setDither(0);


  ScrollingMsg.SetFont(RobotronFontData);
  //ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&ledsm, ledsm.Width(), ScrollingMsg.FontHeight() + 1, 0, 5);
  ScrollingMsg.SetText((unsigned char *)TxtRainbowL, sizeof(TxtRainbowL) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
}

int counter = 0;

void loop() 
{
  counter = encoder.getCount();
  if(counter < 0) {
    counter = 0;
    encoder.setCount(counter);
  } else if(counter >= kMatrixHeight) {
    counter = kMatrixHeight;
    encoder.setCount(counter);
  }
  Serial.println((int)counter);

  //FastLED.clear();

  if (ScrollingMsg.UpdateText() == -1)
    ScrollingMsg.SetText((unsigned char *)TxtRainbowL, sizeof(TxtRainbowL) - 1);

  // copy data
  for( int x = 0; x < kMatrixWidth; ++x) {
    for( int y = 0; y < kMatrixHeight; ++y) {
      leds[XY(x, (kMatrixHeight-1) - y)] = ledsm(x,y);
    }
  }

  FastLED.show();
  FastLED.delay(25);
}
