/*
Copyright (c) 2019 Shajeeb TM

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <FastLED.h>

#define SAMPLES 256            //Must be a power of 2
#define UPDATES_PER_SECOND 144
#define LED_PIN     5
#define NUM_LEDS    300
#define BRIGHTNESS  255
#define AMP_FAC 3
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

uint8_t * brightness = malloc(sizeof(uint8_t));
uint16_t * valley = malloc(sizeof(uint16_t));
uint16_t * peak = malloc(sizeof(uint16_t));
uint16_t * val = malloc(sizeof(uint16_t));

static const CRGBPalette16 currentPalette= RainbowColors_p;
static const TBlendType currentBlending = LINEARBLEND;
 
int music = 0;

void setup() {
    Serial.begin(115200);
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );            // wait to get reference voltage stabilized
}
 
void loop() {
   // ++ Sampling
   if (Serial.available() > 0) {
    // read the incoming byte:
    int temp = Serial.read();
    Serial.print(temp);
    if (temp > 47) {
    music = temp;
    }
   }
     static double startIndex = 0.0;
   if (music > 48) {
   *peak = 0;
   *valley = 1023;
   for(int i=0; i<SAMPLES; i++)
    {
      *val = (uint16_t) analogRead(A0);
      if (*val < *valley) {
        *valley = *val;
      }
      if (*val > *peak) {
        *peak = *val;
      }
                             
    }
    
    *brightness = (*peak - *valley)*AMP_FAC;
    //Serial.println(*brightness);
     // -- send to display according measured value 
    startIndex = startIndex + 3; /* motion speed */
    //FastLED.delay(1000 / UPDATES_PER_SECOND);
} else {
  *brightness = (uint8_t) BRIGHTNESS;

    startIndex = startIndex + 0.25; /* motion speed */
}
    
    FillLEDsFromPaletteColors( (uint8_t) startIndex);
    FastLED.setBrightness( *brightness);
    FastLED.show();
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
