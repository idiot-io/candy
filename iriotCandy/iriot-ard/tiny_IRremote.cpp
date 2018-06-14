/*
 * tiny_IRremote
 * Version 0.2 July, 2016
 * Christian D'Abrera
 * Fixed what was originally rather broken code from http://www.gammon.com.au/Arduino/
 * ...itself based on work by Ken Shirriff.
 *
 * This code was tested for both sending and receiving IR on an ATtiny85 DIP-8 chip.
 * IMPORTANT: IRsend only works from PB4 ("pin 4" according to Arduino). You will need to 
 * determine which physical pin this corresponds to for your chip, and connect your transmitter
 * LED there.
 *
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 */

#include "tiny_IRremote.h"
#include "tiny_IRremoteInt.h"

// Provides ISR
#include <avr/interrupt.h>

volatile irparams_t irparams;


void IRsend::sendNEC(unsigned long data, int nbits)
{
  enableIROut(38);
  mark(NEC_HDR_MARK);
  space(NEC_HDR_SPACE);
  for (int i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      mark(NEC_BIT_MARK);
      space(NEC_ONE_SPACE);
    } 
    else {
      mark(NEC_BIT_MARK);
      space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  mark(NEC_BIT_MARK);
  space(0);
}


void IRsend::mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.
  GTCCR |= _BV(COM1B1); // Enable pin 3 PWM output (PB4 - Arduino D4)
  delayMicroseconds(time);
}

/* Leave pin off for time (given in microseconds) */
void IRsend::space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.
  GTCCR &= ~(_BV(COM1B1)); // Disable pin 3 PWM output (PB4 - Arduino D4)
  delayMicroseconds(time);
}

void IRsend::enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (PB4 - Arduino D4) (OC1B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER1 is used in fast PWM mode, with OCR1Ccontrolling the frequency and OCR1B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 8MHz / (2 * OCR1C)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.

  
  // Disable the Timer1 Interrupt (which is used for receiving IR)
  TIMSK &= ~_BV(TOIE1); //Timer1 Overflow Interrupt

  
  pinMode(4, OUTPUT);  // (PB4 - Arduino D4 - physical pin 3)
  digitalWrite(4, LOW); // When not sending PWM, we want it low
  

  // CTC1 = 1: TOP value set to OCR1C
  // CS = 0001: No Prescaling
  TCCR1 = _BV(CTC1) | _BV(CS10);
  
  // PWM1B = 1: Enable PWM for OCR1B
  GTCCR = _BV(PWM1B);

  // The top value for the timer.  The modulation frequency will be SYSCLOCK / OCR1C.
  OCR1C = SYSCLOCK / khz / 1000;
  OCR1B = OCR1C / 3; // 33% duty cycle

}

// TIMER1 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts
ISR(TIM1_OVF_vect)
{
  RESET_TIMER1;

  uint8_t irdata = (uint8_t)digitalRead(irparams.recvpin);

  irparams.timer++; // One more 50us tick
  if (irparams.rawlen >= RAWBUF) {
    // Buffer overflow
    irparams.rcvstate = STATE_STOP;
  }
  switch(irparams.rcvstate) {
  case STATE_IDLE: // In the middle of a gap
    if (irdata == MARK) {
      if (irparams.timer < GAP_TICKS) {
        // Not big enough to be a gap.
        irparams.timer = 0;
      } 
      else {
        // gap just ended, record duration and start recording transmission
        irparams.rawlen = 0;
        irparams.rawbuf[irparams.rawlen++] = irparams.timer;
        irparams.timer = 0;
        irparams.rcvstate = STATE_MARK;
      }
    }
    break;
  case STATE_MARK: // timing MARK
    if (irdata == SPACE) {   // MARK ended, record time
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = STATE_SPACE;
    }
    break;
  case STATE_SPACE: // timing SPACE
    if (irdata == MARK) { // SPACE just ended, record it
      irparams.rawbuf[irparams.rawlen++] = irparams.timer;
      irparams.timer = 0;
      irparams.rcvstate = STATE_MARK;
    } 
    else { // SPACE
      if (irparams.timer > GAP_TICKS) {
        // big SPACE, indicates gap between codes
        // Mark current code as ready for processing
        // Switch to STOP
        // Don't reset timer; keep counting space width
        irparams.rcvstate = STATE_STOP;
      } 
    }
    break;
  case STATE_STOP: // waiting, measuring gap
    if (irdata == MARK) { // reset gap timer
      irparams.timer = 0;
    }
    break;
  }

}


