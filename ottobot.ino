// No Quarz required version for controlling a AY-3-8910 sound chip with the Arduino
// Author: Matio Klingemann http://incubator.quasimondo.com

// Credits:
// This is combination of code found in this tutorial:
// http://playground.arduino.cc/Main/AY38910
// (which is based on this http://kalshagar.wikispaces.com/Arduino+and+a+YMZ294)
// and code for generating a 2 MHz clock signal found here:
// http://forum.arduino.cc/index.php/topic,62964.0.html

////Pin connected to Data in (DS) of 74HC595
const int dataPin = 5;
//Pin connected to latch pin (ST_CP) of 74HC595
const int latchPin = 6;
//Pin connected to clock pin (SH_CP) of 74HC595
const int clockPin = 7;

const int pinBC1 =  13;
const int pinBCDIR =  12;

// 2mz clock pin
const int freqOutputPin = 11;   // OC2A output pin for ATmega328 boards

const int prescale  = 1;
const int ocr2aval  = 3;
const float period    = 2.0 * prescale * (ocr2aval+1) / (F_CPU/1.0e6);
const float freq      = 1.0e6 / period;

//modes
const int mode[7][12]{
  {0,2,4,5,7,9,11,12,14,16,17,19}, //major
  {0,2,3,5,7,9,10,12,14,15,17,19}, //dorian
  {0,1,3,5,7,8,10,12,13,15,17,19}, //phyrgian
  {0,2,4,6,7,9,11,12,14,16,18,19}, //lydian
  {0,2,4,5,7,9,10,12,14,16,17,19}, //mixolydian
  {0,2,3,5,7,8,10,12,14,15,17,19}, //aeolian/minor
  {0,1,3,5,6,8,10,12,13,15,17,18}, //locrian
};

int tempo;
int interval = 0;
int prog = 0;
int modeselect = 0;
int key = 0;
int songLength = 0;
int songWrite[7];
int song[] = {1,2,3,4,5,6,7};
boolean ascend = 1;
int keyWait = 0;
int modeWait = 0;

int tp[] = {//MIDI note number
  15289, 14431, 13621, 12856, 12135, 11454, 10811, 10204,//0-o7
  9631, 9091, 8581, 8099, 7645, 7215, 6810, 6428,//8-15
  6067, 5727, 5405, 5102, 4816, 4545, 4290, 4050,//16-23
  3822, 3608, 3405, 3214, 3034, 2863, 2703, 2551,//24-31
  2408, 2273, 2145, 2025, 1911, 1804, 1703, 1607,//32-39
  1517, 1432, 1351, 1276, 1204, 1136, 1073, 1012,//40-47
  956, 902, 851, 804, 758, 716, 676, 638,//48-55
  602, 568, 536, 506, 478, 451, 426, 402,//56-63
  379, 358, 338, 319, 301, 284, 268, 253,//64-71
  239, 225, 213, 201, 190, 179, 169, 159,//72-79
  150, 142, 134, 127, 119, 113, 106, 100,//80-87
  95, 89, 84, 80, 75, 71, 67, 63,//88-95
  60, 56, 53, 50, 47, 45, 42, 40,//96-103
  38, 36, 34, 32, 30, 28, 27, 25,//104-111
  24, 22, 21, 20, 19, 18, 17, 16,//112-119
  15, 14, 13, 13, 12, 11, 11, 10,//120-127
  0//off
};

//keyboard
int keys[] = {49,50,51,52,53,54,55};

void setup()
{
   //init pins
    pinMode(latchPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(pinBC1, OUTPUT);
    pinMode(pinBCDIR, OUTPUT);
    pinMode(freqOutputPin, OUTPUT);

  //reset pin
    pinMode(2,OUTPUT);
    digitalWrite(2,HIGH);
    digitalWrite(2,LOW);
    delayMicroseconds(5); // more than 500us required.
    digitalWrite(2,HIGH);

    init2MhzClock();

    set_mix( true, true, true, false, false, false );
    set_chA_amplitude(4,false);
    set_chB_amplitude(4,false);
    set_chC_amplitude(4,false);

    Serial.begin(9600);

}


void loop() {

    tempo = (20* (1 + ( analogRead(0) / 70)));

//    note = 40 + mode[modeselect][prog];

    set_envelope(true,true,true,false,analogRead(0));

    set_chA_amplitude(6,false);
    set_chB_amplitude(6,false);
    set_chC_amplitude(6,false);
    int note = 50+mode[modeselect][song[interval]];
    note_chA(note);
    note_chB(note+4);
    note_chC(note+7);
    modecheck();
    set_chA_amplitude(0,false);
    set_chB_amplitude(0,false);
    set_chC_amplitude(0,false);





    if(interval==6){
      ascend = 0;
      prog++;
    }

   if(interval == 0){
     ascend = 1;
   }

   if (ascend){
      interval++;
   }

   else{
     interval--;
   }

  if(prog > 7){
    prog = 0;
   }

}

void modecheck() {

//tempo
  for (int n=0; n < tempo; n++){

//check modeswitch button
      if(digitalRead(3) && modeWait <1){
        modeselect++;
        Serial.println("modeswitched");
        modeWait = 200;
        if (modeselect == 6){
          modeselect = 0;
        }
      }
//check for keyboard input A S or D
      if ((Serial.available() > 0) && keyWait < 1) {
        char inChar = Serial.read();
        key = findkey(inChar);

//if not too fast pressed and is digit 1_7 add note to song
        if (key > 0){
          songWrite[songLength] = key;
          songLength++;
          keyWait = 200;
        }
      }

        if (songLength == 7){
          for (int i = 0; i < 7; i++) {
            song[i] = songWrite[i];
          }
          songLength = 0;
          Serial.println("Song written: ");
            for (int i = 0; i < 7; i++) {
              Serial.print(song[i]);
              Serial.print(" ");
            }
        }

      delay(1);
      keyWait--;
      modeWait--;
  }
}

void  init2MhzClock()
{
    // Set Timer 2 CTC mode with no prescaling.  OC2A toggles on compare match
    //
    // WGM22:0 = 010: CTC Mode, toggle OC
    // WGM2 bits 1 and 0 are in TCCR2A,
    // WGM2 bit 2 is in TCCR2B
    // COM2A0 sets OC2A (arduino pin 11 on Uno or Duemilanove) to toggle on compare match
    //
    TCCR2A = ((1 << WGM21) | (1 << COM2A0));

    // Set Timer 2  No prescaling  (i.e. prescale division = 1)
    //
    // CS22:0 = 001: Use CPU clock with no prescaling
    // CS2 bits 2:0 are all in TCCR2B
    TCCR2B = (1 << CS20);

    // Make sure Compare-match register A interrupt for timer2 is disabled
    TIMSK2 = 0;
    // This value determines the output frequency
    OCR2A = ocr2aval;
}

void set_mix( boolean chA_tone,boolean chB_tone,boolean chC_tone,boolean chA_noise,boolean chB_noise,boolean chC_noise )
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

void set_chA_amplitude(int amplitude, boolean useEnvelope )
{
   write_data(8, (amplitude & 0xf) | (useEnvelope != true ? 0 : B00010000 ) );
}

void set_chB_amplitude(int amplitude, boolean useEnvelope )
{
  write_data(9, (amplitude & 0xf) | (useEnvelope != true ? 0 : B00010000 ) );
}

void set_chC_amplitude(int amplitude, boolean useEnvelope )
{
  write_data(10, (amplitude & 0xf) | (useEnvelope != true ? 0: B00010000 ) );
}

void set_envelope( boolean hold, boolean alternate, boolean attack, boolean cont, unsigned long frequency )
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

/* void recvData() {
 if (Serial.available() > 0) {
 input = Serial.read();
 newData = 1;
  }
}
*/

 int findkey(char inKey){
    int i;
    for(i = 0; i < 7; i++){
        if(keys[i] == inKey) return (i+1);
    }
    return 0;
  }
