/*
   words http://idiot.io
   code https://github.com/shenkarSElab/IRiot

   pin out attiny85
   NC      -1+----+8- VCC
   Touch   -2|*   |7- NC
   IRsend  -3|    |6- LED
   GND     -4+----+5- NC

*/

#include "tiny_IRremote.h" 

#define ledPin    0     // pin5 / PB0
IRsend irsend;          // pin3 / PB4 / Digital 4 / TIMER1 output compare unit
#define YOUR_NEC_CODE 9000

int ledState = LOW;

/*
   ======================================================
   TinyTouch headers
   https://github.com/cpldcpu/TinyTouchLib
   see lib configuration at the readme and .h file
*/
#include <util/delay.h>
#include <avr/io.h>

// Setting the "on" value lower will make the touch button more sensitive.
// Setting the "off" value higher will make the touch button less likely
// to be "stuck". Too high values can lead to oscillations.
#define touch_threshold_on 60
#define touch_threshold_off 20
#define touch_timeout 255
//#define touch_timeout 0    // turn off timeout functionality

// Define pins to use for the reference input and the touch button
// The reference pin is used to charge or discharge the internal
// sample&hold capacitor. This pin is used in output mode and should
// not be shorted to VCC or GND externally.
// The sense pin is connected to the touch-button. To improve noise immunity
// a series resistor can be used.

// The pin number must match the corresponding analog input number ADCx.
// Default port is PORTB. (ATtiny 5/10/13/25/45/85)
#define tt_refpin 2    // Use PB2 as reference pin
#define tt_refadc 1   // Use ADC1 as reference ADC input
#define tt_sensepin 3 // Use PB3 as sense pin
#define tt_senseadc 3 // Use ADC3 as sense ADC input

// The sense function evaluates the button state and performs internal
// housekeeping. It should be polled at least 30 times per second to
// update the internal logic. Please note that each call performs 32
// analog to digital conversions with active waiting. This may take
// several ms.

// Possible return values are:
//    tt_off=0  No touch sensed
//    tt_on   Touch button is active and touch is sensed.
//    tt_push   Touch button is pushed. Use this to initiate one time events.
//    tt_release  Touch button is released. Use this to initiate one time events.
//    tt_timeout  Touch button has been active too long and internal bias was reset.

enum {tt_off = 0, tt_on, tt_push, tt_release, tt_timeout};
// Internal function to read the adc input
uint8_t tinytouch_adc(void);

uint16_t bias;
uint8_t touch;
#if touch_timeout>0
uint8_t timer;
#endif
//============================================================

int netcode;

void setup() {
	
	//////////////
  tinytouch_init();
  /////////////////

  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  netcode=random(314);
}

void loop() {

  uint8_t touchstate = tinytouch_sense();
  if (touchstate == tt_push) {
    ledState = HIGH;
	
    //irsend.sendNEC(YOUR_NEC_CODE, 32); // sending the nec code
    irsend.sendNEC(netcode, 32); // sending the nec code
	
    digitalWrite(ledPin, ledState);
  }
  if (touchstate == tt_release) {
    ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
  
}


/*
   =================================================
   TinyTouch functions
   =================================================
*/
void  tinytouch_init(void) {

#ifndef __AVR_ATtiny13__
  PRR   &= ~_BV(PRADC);
#endif
  ADCSRA  = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1); // Enable ADC, Set prescaler to 64

  bias = tinytouch_adc() << 8;
  touch = 0;
}

uint8_t tinytouch_sense(void) {
  uint8_t i;
  uint16_t tmp;
  int16_t delta;

  tmp = 0;
  for (i = 0; i < 16; i++) {
    tmp += tinytouch_adc(); // average 16 samples
    _delay_us(100);
  }

  delta = tmp - (bias >> 4);

  if (!touch) {
    if (delta > touch_threshold_on) {
      touch = 1;
#if touch_timeout>0
      timer = 0;
#endif
      return tt_push;
    }

    // update bias only when touch not active
    bias = (bias - (bias >> 6)) + (tmp >> 2); // IIR low pass
    return tt_off;
  } else {
    if (delta < touch_threshold_off) {
      touch = 0;
      return tt_release;
    }

#if touch_timeout>0
    if (timer == 255) {
      bias = tinytouch_adc() << 8;
      return tt_timeout;
    }
    timer++;
#endif
    return tt_on;
  }
}

uint8_t tinytouch_adc(void) {

  uint8_t dat1, dat2;

  // Precharge Low
  ADMUX = tt_refadc; // connect S/H cap to reference pin
  PORTB |= _BV(tt_refpin);    // Charge S/H Cap
  PORTB &= ~_BV(tt_sensepin);   // Discharge Pad (0)
  DDRB  |= _BV(tt_refpin) | _BV(tt_sensepin);

  _delay_us(32);

  DDRB  &= ~(_BV(tt_sensepin)); // float pad input, note that pull up is off.

#ifdef __AVR_ATtiny10__
  ADMUX = tt_senseadc; // Connect sense input to adc
#else
  ADMUX = tt_senseadc | _BV(ADLAR); // Connect sense input to adc
#endif

  ADCSRA  |= _BV(ADSC); // Start conversion
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA  |= _BV(ADIF); // Clear ADIF

#ifdef __AVR_ATtiny10__
  dat1 = ADCL;
#else
  dat1 = ADCH;
#endif

  // Precharge High
  ADMUX  = tt_refadc; // connect S/H cap to reference pin
  PORTB &= ~_BV(tt_refpin);   // Discharge S/H Cap
  PORTB |= _BV(tt_sensepin);    // Charge Pad
  DDRB  |= _BV(tt_refpin) | _BV(tt_sensepin);

  _delay_us(32);

  DDRB  &= ~(_BV(tt_sensepin)); // float pad input input
  PORTB &= ~_BV(tt_sensepin);   // pull up off

#ifdef __AVR_ATtiny10__
  ADMUX = tt_senseadc; // Connect sense input to adc
#else
  ADMUX = tt_senseadc | _BV(ADLAR); // Connect sense input to adc
#endif

  ADCSRA  |= _BV(ADSC); // Start conversion
  while (!(ADCSRA & _BV(ADIF)));
  ADCSRA  |= _BV(ADIF); // Clear ADIF

#ifdef __AVR_ATtiny10__
  dat2 = ADCL;
#else
  dat2 = ADCH;
#endif


  return dat2 - dat1;
}
