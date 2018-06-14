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
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 
 adapted to ATTiny85 TIMER1 by seejaydee
 https://gist.github.com/SeeJayDee/caa9b5cc29246df44e45b8e7d1b1cdc5
 */

#ifndef tiny_IRremote_h
#define tiny_IRremote_h



// Values for decode_type
#define NEC 1
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff


class IRsend
{
public:
  IRsend() {}
  void sendNEC(unsigned long data, int nbits);
  // private:
  void enableIROut(int khz);
  void mark(int usec);
  void space(int usec);
}
;

// Some useful constants

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 76 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif
