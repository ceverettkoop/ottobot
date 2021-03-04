#include "aycontrol.h"
#include <Arduino.h>

void set_mix( bool chA_tone,bool chB_tone,bool chC_tone,bool chA_noise,bool chB_noise,bool chC_noise )
{
   write_data(7, B11000000 |
                   (chC_noise == true ? 0 : B00100000)|
                    (chB_noise == true? 0 : B00010000) |
                    (chA_noise == true ? 0 : B00001000) |
                    (chC_tone == true ? 0 : B00000100) |
                    (chB_tone == true ? 0 : B00000010) |
                    (chA_tone == true ? 0 : B00000001)
   );
}

void set_chA_amplitude(int amplitude, bool useEnvelope )
{
   write_data(8, (amplitude & 0xf) | (useEnvelope != true ? 0 : B00010000 ) );
}

void set_chB_amplitude(int amplitude, bool useEnvelope )
{
  write_data(9, (amplitude & 0xf) | (useEnvelope != true ? 0 : B00010000 ) );
}

void set_chC_amplitude(int amplitude, bool useEnvelope )
{
  write_data(10, (amplitude & 0xf) | (useEnvelope != true ? 0: B00010000 ) );
}

void set_envelope( bool hold, bool alternate, bool attack, bool cont, unsigned long frequency )
{
    write_data(13, (hold == true ? 0 : 1)|
                    (alternate == true? 0 : 2) |
                    (attack == true ? 0 : 4) |
                    (cont == true ? 0 : 8)
                );

    write_data(11,frequency & 0xff );
    write_data(12,(frequency >> 8)& 0xff );

}

void note_chA(int i)
{
  write_data(0x00, tp[i]&0xff);
  write_data(0x01, (tp[i] >> 8)&0x0f);
}

void note_chB(int i)
{
  write_data(0x02, tp[i]&0xff);
  write_data(0x03, (tp[i] >> 8)&0x0f);
}

void note_chC(int i)
{
  write_data(0x04, tp[i]&0xff);
  write_data(0x05, (tp[i] >> 8)&0x0f);
}

void noise(int i)
{
  write_data(0x06, i&0x1f);
}

void mode_latch(){
    digitalWrite(pinBC1, HIGH);
    digitalWrite(pinBCDIR, HIGH);
}

void mode_write(){
    digitalWrite(pinBC1, LOW);
    digitalWrite(pinBCDIR, HIGH);
}

void mode_inactive(){
    digitalWrite(pinBC1, LOW);
    digitalWrite(pinBCDIR, LOW);
}

void write_data(unsigned char address, unsigned char data)
{
  mode_inactive();
  //write address

  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, address);

  //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);

  mode_latch();
  mode_inactive();

  //write data
  mode_write();

  digitalWrite(latchPin, LOW);
  // shift out the bits:
  shiftOut(dataPin, clockPin, MSBFIRST, data);

  //take the latch pin high so the LEDs will light up:
  digitalWrite(latchPin, HIGH);

  mode_inactive();
}
