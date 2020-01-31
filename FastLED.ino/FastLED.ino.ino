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
#include <arduinoFFT.h>
#include <SPI.h>

#define SAMPLES 64            //Must be a power of 2
#define UPDATES_PER_SECOND 100
#define LED_PIN     5
#define NUM_LEDS    300
#define BRIGHTNESS  255
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];
 
double vReal[SAMPLES];
double vImag[SAMPLES];
char data_avgs[NUM_LEDS];
uint8_t brightness = 255;
int yvalue;


static const CRGBPalette16 currentPalette= RainbowColors_p;
static const TBlendType currentBlending = LINEARBLEND;
arduinoFFT FFT = arduinoFFT();                                    // FFT object
 


void setup() {
    Serial.begin(115200);
    
    //ADCSRA = 0b11100101;      // set ADC to free running mode and set pre-scalar to 32 (0xe5)
    //ADMUX = 0b00000000;       // use pin A0 and external voltage reference
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );            // wait to get reference voltage stabilized
}
 
void loop() {
   // ++ Sampling
   Serial.write("sampling\n");
   for(int i=0; i<SAMPLES; i++)
    {
      //while(!(ADCSRA & 0x10));        // wait for ADC to complete current conversion ie ADIF bit set
      //ADCSRA = 0b11110101 ;               // clear ADIF bit so that ADC can do next operation (0xf5)
      int value = ADC - 512 ;
      int val = analogRead(A0);
      Serial.println(val);
      vReal[i]= value/8;                      // Copy to bins after compressing
      vImag[i] = 0;                         
    }
    // -- Sampling
      Serial.write("test2\n");
    // ++ FFT
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);
    // -- FFT
      Serial.println("FFT done");
    
    // ++ re-arrange FFT result to match with no. of columns on display ( xres )
    int step = (SAMPLES/2)/NUM_LEDS; 
    int c=0;
    for(int i=0; i<(SAMPLES/2); i+=step)  
    {
      data_avgs[c] = 0;
      for (int k=0 ; k< step ; k++) {
          data_avgs[c] = data_avgs[c] + vReal[i+k];
      }
      data_avgs[c] = data_avgs[c]/step; 
      c++;
    }
    // -- re-arrange FFT result to match with no. of columns on display ( xres )

    
    // ++ send to display according measured value 
    for(int i=0; i<NUM_LEDS; i++)
    {
      data_avgs[i] = constrain(data_avgs[i],0,80);            // set max & min values for buckets
      data_avgs[i] = map(data_avgs[i], 0, 80, 0, 255);        // remap averaged values to yres
      yvalue=data_avgs[i];       // for left to right
     }
     brightness=(uint8_t) data_avgs[3];
     // -- send to display according measured value 
    static uint8_t startIndex = 0;
    startIndex = startIndex + 2; /* motion speed */
    
    FillLEDsFromPaletteColors( startIndex);
    FastLED.setBrightness( brightness );
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
