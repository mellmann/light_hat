

#include <Arduino.h>

#include <ezButton.h>  // the library to use for SW pin
#include <ESP32Encoder.h>

#include <Adafruit_BNO055.h>

#define CLK_PIN 5  // ESP32 pin GPIO25 connected to the rotary encoder's CLK pin
#define DT_PIN  18 // ESP32 pin GPIO26 connected to the rotary encoder's DT pin
#define SW_PIN  19 // ESP32 pin GPIO27 connected to the rotary encoder's SW pin


ESP32Encoder encoder;
ezButton button(SW_PIN);

Adafruit_BNO055 bno;

#include <FastLED.h>
// Here's how to control the LEDs from any two pins:
#define DATAPIN    13


#include "HatMatrix.h"

// text stuff
#include "LEDMatrix.h"
#include "LEDText.h"
#include "FontRobotron.h"
#include "FontMatrise.h"
#include "FontAtari.h"

#include "Ball.h"
#include "Fire.h"

#define NUMPIXELS 600 // Number of LEDs in strip
#define MATRIX_WIDTH   69
#define MATRIX_HEIGHT  18

// needed for text matrix
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX


cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> ledsm;
cLEDText ScrollingMsg;
const unsigned char TxtRainbowL[] = { EFFECT_HSV "\x00\xff\xff" "H" EFFECT_HSV "\x20\xff\xff" "+" EFFECT_HSV "\x40\xff\xff" "V" EFFECT_HSV "\x60\xff\xff" "=" EFFECT_HSV "\x80\xff\xff" "<3"};



// dimensions of the hat matrix
// NOTE: The distance between the LEDs on the LED-strip is 16 mm.
//   The lines are placed with an offes of half a distance between the LEDs.
//   Between the LES we consider to be 'invisible' pixel.
//   This leads to the following matrix of pixels, with a horizontal distance of 8mm between the pixels
//     - LED-pixels:       [O]
//     - invisible pixels: [ ]
//       
//   [O][ ][O][ ][O][ ][O][ ]
//   [ ][O][ ][O][ ][O][ ][O]
//   [O][ ][O][ ][O][ ][O][ ]
//   [ ][O][ ][O][ ][O][ ][O]
//   [O][ ][O][ ][O][ ][O][ ]
//
// NOTE: maybe for future improvements: 8*3/2 = 12
const float px_dist_x = 8;  // mm
const float px_dist_y = 12; // mm

typedef HatMatrix<NUMPIXELS, MATRIX_WIDTH, MATRIX_HEIGHT> MyHatMatrix;
MyHatMatrix matrix(px_dist_x, px_dist_y);

// fire animation
SpaceBalls<MyHatMatrix> spaceBalls(matrix);

// fire animation
Fire<MyHatMatrix> fire(matrix);



void setup() 
{  
  Serial.begin(115200);

  //encoder.attachFullQuad ( DT_PIN, CLK_PIN );
  encoder.attachSingleEdge ( DT_PIN, CLK_PIN );
  encoder.setCount ( 0 );

  button.setDebounceTime(50);  // set debounce time to 50 milliseconds

  
  Serial.println("Initializing IMU...");
  if(!bno.begin()) {
      Serial.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  } else {
    Serial.println("BNO055 connected!");
    delay(1000); // FIXME: do we need that?
    bno.setExtCrystalUse(true);
  }
  
  //FastLED.addLeds<WS2812B, DATAPIN, GRB>(leds, NUMPIXELS);
  FastLED.addLeds<WS2812B, DATAPIN, GRB>(matrix.getLEDArray(), matrix.num_pixels());
  FastLED.setBrightness(255);
  FastLED.setDither(0);


  ScrollingMsg.SetFont(RobotronFontData);
  //ScrollingMsg.SetFont(AtariFontData);
  
  //ScrollingMsg.SetFont(MatriseFontData);
  ScrollingMsg.Init(&ledsm, ledsm.Width(), ScrollingMsg.FontHeight() + 1, 0, 5);
  ScrollingMsg.SetText((unsigned char *)TxtRainbowL, sizeof(TxtRainbowL) - 1);
  ScrollingMsg.SetTextColrOptions(COLR_RGB | COLR_SINGLE, 0xff, 0x00, 0xff);
  ScrollingMsg.UpdateText();
}


float last_angle = 0;
float angle_diff = 0; // angle movement since last update

void calculate_angle() 
{
  // calculate angle
  imu::Vector<3> euler = bno.getVector(Adafruit_BNO055::VECTOR_EULER);

  float new_angle = euler.x();
  angle_diff = new_angle - last_angle;
  if(angle_diff > 180) {
    angle_diff -= 360;
  } else if(angle_diff < -180) {
    angle_diff += 360;
  }

  last_angle = new_angle;
  Serial.println(last_angle);
}



int x_offset = 0;

void update_text() 
{
  // copy data
  for( int x = 0; x < matrix.width(); ++x) {
    for( int y = 0; y < matrix.height(); ++y) {
      int x_rotated = (x + x_offset) % matrix.width();
      matrix.pixel( (matrix.width()-1) - x_rotated, y) = ledsm(x,y);
    }
  }

  x_offset = x_offset-1;
  if (x_offset < 0) { 
    x_offset += matrix.width();
  }
}


int getEncoder() 
{
  const int maxCounter = 18;
  int counter = encoder.getCount();
  if(counter < 0) {
    counter = 0;
    encoder.setCount(counter);
  } else if(counter >= maxCounter) {
    counter = maxCounter;
    encoder.setCount(counter);
  }
  //Serial.println(counter);
  return counter;
}


int button_count_old = 0;
int app_state = 0;
unsigned long app_state_last_change_time = 0;

void loop() 
{
  button.loop();
  calculate_angle();
  int counter = getEncoder();
  
  unsigned long app_state_time = millis() - app_state_last_change_time;
  

  int button_count = button.getCount();
  if((app_state_time > 1000 && button_count != button_count_old) || app_state_time > 10000) {
    app_state = (app_state + 1) % 4;
    app_state_last_change_time = millis();
  }
  button_count_old = button_count;


  switch(app_state) {
    case 0: 
      FastLED.clear();
      FastLED.setBrightness(255);
      counter = 10;
      spaceBalls.draw_snow(counter*10); 
      spaceBalls.update_imu(angle_diff); 
      break;
    case 1:
      FastLED.clear();
      FastLED.setBrightness(255);
      counter = 10;
      spaceBalls.draw_snow(counter*10); 
      spaceBalls.update_bouncing();
      break;
    case 2: 
      FastLED.clear();
      FastLED.setBrightness(64);
      fire.update(counter);
      break;
    case 3: 
      FastLED.clear();
      FastLED.setBrightness(64);
      update_text();
      break;
  }

  /*
  FastLED.clear();
  gfx.drawRect(7,7,6,6,8);
  gfx.println("A");
  gfx.print(false);
  */
  
  FastLED.show();
  FastLED.delay(25);
}
