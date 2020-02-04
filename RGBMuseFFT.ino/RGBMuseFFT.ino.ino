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

#define SAMPLES 64            //Must be a power of 2
#define UPDATES_PER_SECOND 144
#define LED_PIN     5
#define NUM_LEDS    300
#define BRIGHTNESS  155
#define AMP_FAC 3
#define SPEC_PART 2
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define SILENCE_THRESHOLD 25
#define SILENCE_THRESHOLD_VALUE 8
#define FFT_OFFSET 2
#define B_OFFSET 80

CRGB leds[NUM_LEDS];
static const double PROGMEM weights[SAMPLES/SPEC_PART-FFT_OFFSET] = {0.06666667, 0.06436782, 0.06206897, 0.05977011, 0.05747126,
       0.05517241, 0.05287356, 0.05057471, 0.04827586, 0.04597701,
       0.04367816, 0.04137931, 0.03908046, 0.03678161, 0.03448276,
       0.03218391, 0.02988506, 0.02758621, 0.02528736, 0.02298851,
       0.02068966, 0.0183908 , 0.01609195, 0.0137931 , 0.01149425,
       0.0091954 , 0.00689655, 0.0045977 , 0.00229885, 0.        };

uint8_t * brightness = malloc(sizeof(uint8_t));
double vReal[SAMPLES];
double vImag[SAMPLES];
uint16_t data_avgs[NUM_LEDS];
byte iter=0;
arduinoFFT FFT = arduinoFFT();  

static const CRGBPalette16 currentPalette= RainbowColors_p;
static const TBlendType currentBlending = LINEARBLEND;
int music_mode = 49; 
int music = 0;
double sum = 0; 
void setup() {
    //Serial.begin(115200);
    pinMode(A0, INPUT);
    delay( 3000 ); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );            // wait to get reference voltage stabilized
}
 
void loop() {
   // ++ Sampling
   //if (Serial.available() > 0) {
    // read the incoming byte:
    //int temp = Serial.read();
    //if (temp > 47) {
    //music_mode = temp;
    //music = temp;
    //}
   //}
     static double startIndex = 0.0;



   for(int i=0; i<SAMPLES; i++)
    {
    vReal[i]= (double) analogRead(A0);
    vImag[i]=0;
                             
    }

    //FFT
    FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);


    
   if (music==49) {
      double val=0;
      for(int i=FFT_OFFSET; i<SAMPLES/SPEC_PART; i++)
      {
        val+=vReal[i];//*weights[i];                       
      }
    *brightness = val*AMP_FAC/(SAMPLES/SPEC_PART-FFT_OFFSET);
    } else if ( music ==50) {

      
       for(int i=0; i<NUM_LEDS; i++)  
      {
        leds[i]= CHSV( data_avgs[i], 255, data_avgs[i]);
      }
      *brightness=255;
    } else {
      double val=0;
      for(int i=FFT_OFFSET; i<SAMPLES/SPEC_PART; i++)
      {
        val+=vReal[i];                       
      }
    *brightness = val*AMP_FAC/(SAMPLES/SPEC_PART-FFT_OFFSET);
    }

    
    
    Serial.println(*brightness);
    if (iter == SILENCE_THRESHOLD) {
      if (sum/SILENCE_THRESHOLD < SILENCE_THRESHOLD_VALUE*AMP_FAC) {
        music=48;
        //Serial.println("music off");

      }
      else {
        music=music_mode;
        //Serial.println("music on");
      }
      sum =0;
      iter=0;
    }
    else {
      sum+=*brightness;
    }
    iter++;
if (music > 48) {
     // -- send to display according measured value 
    startIndex = startIndex + 3; /* motion speed */
    //FastLED.delay(1000 / UPDATES_PER_SECOND);
} else {
  *brightness = (uint8_t) BRIGHTNESS;

    startIndex = startIndex + 0.75; /* motion speed */
}
    
    FillLEDsFromPaletteColors( (uint8_t) startIndex);
    FastLED.setBrightness( constrain(*brightness+B_OFFSET,0,255));
    FastLED.show();
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    
    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
        colorIndex += 3;
    }
}
