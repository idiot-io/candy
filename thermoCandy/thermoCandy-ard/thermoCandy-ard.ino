
/*
  ThermoCandy
   for kids only

   words http://idiot.io
   code https://github.com/shenkarSElab/IRiot/thermoCandy

   pin out attiny85
   NC      -1+----+8- VCC
   NC      -2|*   |7- NC
   Thermo  -3|    |6- Pixel
   GND     -4+----+5- Serial (debug)
   http://drazzy.com/e/img/PinoutT85a.jpg
*/

///////////////////////////////////////////////
///https://forum.arduino.cc/index.php?topic=112013.msg841582#msg841582
#include "SendOnlySoftwareSerial.h"
SendOnlySoftwareSerial mySerial (0);  // Tx pin , PB0, pin5

////////////////////////////////////////////////
//https://github.com/adafruit/Adafruit_NeoPixel
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
#define pixelPin 1 //PB1, pin6
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(1, pixelPin, NEO_GRB + NEO_KHZ800);

///////////////////////////////
// https://github.com/MajenkoLibraries/Average
#include "Average.h"
Average<float> ave(30);
int sensorPin = A2; //pin 3

void setup() {
  mySerial.begin(9600);

  pixels.begin();
}

void loop() {

  int analogValue = analogRead(sensorPin);
  long temperature = (readVcc() / 1000.0) * analogValue * 100.0 / 1024;

  ave.push(temperature);
  float average = ave.mean();


  byte red, green, blue;
  if (average > 20  && average < 35) {
    green = mapfloat(average, 20, 37, 0, 255);
    blue = 255 - red;
    red = 0;
  } else if (average >= 35  && average < 36.5) {
    red = mapfloat(average, 24, 37, 0, 255);
    green = 255 - red;
    blue = 0;
  } else if (average >= 36.5) {
    red = mapfloat(average, 36.5, 37, 0, 255);
    green = 0;
    blue = 0;
  }

  pixels.setPixelColor(0, pixels.Color(red, green, blue));
  pixels.show();

  // And show some interesting results.
  mySerial.print(ave.mean());
  mySerial.print(" ");
  mySerial.print(temperature);

  mySerial.print(" ");
  mySerial.print(red);
  mySerial.print(" ");
  mySerial.print(green);
  mySerial.print(" ");
  mySerial.println(blue);
  /*
    for (int i = 0; i < 255; i++) {
    pixels.setPixelColor(0, pixels.Color(i, 255 - i, 0));
    pixels.show();
    delay(10);
    }
  */

  delay(10);


}


// This code snippet is made by Tinkerit. Open Source license. Slightly modified by DavidJWatts
long readVcc() {
  // Read 1.1V reference against AVcc
  long result;
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(MUX3) | _BV(MUX2);

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return result;
}

float mapfloat(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (float)(x - in_min) * (out_max - out_min) / (float)(in_max - in_min) + out_min;
}

