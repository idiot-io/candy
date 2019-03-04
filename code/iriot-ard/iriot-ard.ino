/*
   words http://idiot.io
   code https://github.com/shenkarSElab/IRiot

   pin out attiny85
  // https://i.imgur.com/935bRjk.png
  // wakeup code from nick gammon
  // http://www.gammon.com.au/forum/?id=11488&reply=9#reply9
            +-\/-+
           1|    |8 VCC
   WAKEUP  2|    |7
   IRsend  3|    |6 LED
   GND     4|    |5
            +----+

  //                  +-\/-+
  // Ain0 (D 5) PB5  1|    |8  Vcc
  // INT3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
  // INT4 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
  //            GND  4|    |5  PB0 (D 0) pwm0
  //                  +----+

*/
#include <avr/sleep.h>    // Sleep Modes
#include <avr/power.h>    // Power management
const byte SWITCH = 3; // pin 2 / PCINT3

#include "tiny_IRremote.h"

#define LED    1     // pin6 / PB1
IRsend irsend;          // pin3 / PB4 / TIMER1 output compare unit
#define YOUR_NEC_CODE 9004

int ledState = LOW;

ISR (PCINT0_vect)
{
  // do something interesting here
}

int netcode;

void setup() {

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  //wakeup
  pinMode (SWITCH, INPUT);
  digitalWrite (SWITCH, HIGH);  // internal pull-up

  // pin change interrupt
  PCMSK  |= bit (PCINT3);  // want pin D3 / pin 2
  GIFR   |= bit (PCIF);    // clear any outstanding interrupts
  GIMSK  |= bit (PCIE);    // enable pin change interrupts

  netcode = random(314);
}

void loop() {

  //https://www.analysir.com/blog/2015/09/01/simple-infrared-pwm-on-arduino-part-3-hex-ir-signals/
  digitalWrite(LED, HIGH);
  irsend.sendNEC(YOUR_NEC_CODE, 32); // sending the nec code
  delay(200);
  digitalWrite(LED,LOW);
  
  goToSleep();
}

void goToSleep ()
{
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  ADCSRA = 0;            // turn off ADC
  power_all_disable ();  // power off ADC, Timer 0 and 1, serial interface
  sleep_enable();
  sleep_cpu();
  sleep_disable();
  power_all_enable();    // power everything back on
}  // end of goToSleep
