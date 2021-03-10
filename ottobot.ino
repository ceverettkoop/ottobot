//Ottobot AY38910 + Arduino Tracker
// Credits:
// Based on code from Matio Klingemann http://incubator.quasimondo.com
// which was in turn based on:
// http://playground.arduino.cc/Main/AY38910
// and code for generating a 2 MHz clock signal found here:
// http://forum.arduino.cc/index.php/topic,62964.0.html

#include "aycontrol.h"

//keyboard inputs defined as ASCII 1-7

int song[16][4];
int noteIn = 0;
int noteOut = 0;
int measure = 0;
int measureIn = 0;
int modeselect = 0;
int songWrite[4];
int keyWait = 0;
int volumeToggle = 0;
int volume = 0;

void setup() {

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

//  set_envelope(true,true,true,false,500);
    Serial.begin(9600);

// write default song 16 measures
    for (int i = 0, n = 0; i < 16; ){
        song[i][n] = random(0,7);
        n++;
        if (n == 4){
          n = 0;
          i++;
        }
      }
}

void loop() {

  const int mode[7][12]{
    {0,2,4,5,7,9,11,12,14,16,17,19}, //major
    {0,2,3,5,7,9,10,12,14,15,17,19}, //dorian
    {0,1,3,5,7,8,10,12,13,15,17,18}, //phyrgian
    {0,2,4,6,7,9,11,12,14,16,18,19}, //lydian
    {0,2,4,5,7,9,10,12,14,16,17,19}, //mixolydian
    {0,2,3,5,7,8,10,12,14,15,17,19}, //aeolian/minor
    {0,1,3,5,6,8,10,12,13,15,17,18}, //locrian
  };

//if playing set volume 6 else 0

  if (volumeToggle){
      if (volume == 0){
        volume = 6;
      }
        else{
          volume = 0;
        }

      set_chA_amplitude(volume,false);
      set_chB_amplitude(volume,false);
      set_chC_amplitude(volume,false);

      volumeToggle = 0;
  }

//play major chord in song

    int pitch = 50+mode[modeselect][song[measure][noteOut]];
    Serial.println(song[measure][noteOut]);
    note_chA(pitch);
    note_chB(pitch+4);
    note_chC(pitch+7);

//get input status
    modeCheck();

//end
    set_chA_amplitude(0,false);
    set_chB_amplitude(0,false);
    set_chC_amplitude(0,false);

    noteOut++;
    measure++;

    if (noteOut == 5){
      noteOut = 0;
    }

    if (measure == 17){
      measure = 0;
    }
}


void modeCheck() {

//get tempo from dial and set as loop limit
  int tempo = (20* (1 + ( analogRead(0) / 70)));
  for (int n=0; n < tempo; n++){

//check for keyboard input matching allowable keys
      if ((Serial.available() > 0) && keyWait < 1) {
        char inChar = Serial.read();
        int key = findKey(inChar);
//if not too fast pressed and is digit 1_7 add note to song
        if (key){
          songWrite[noteIn] = (key - 1);
          noteIn++;
          keyWait = 200;
        }
      }
//when four notes have been written write measure
        if (noteIn == 4){
          for (int i = 0; i < 4; i++) {
            song[measureIn][i] = songWrite[i];
          }
          noteIn = 0;

          Serial.println("Measure written: ");

          for (int i = 0; i < 4; i++) {
            Serial.print(song[measureIn][i]);
            Serial.print(" ");
            }

          measureIn++;
          if (measureIn == 16){
            measureIn = 0;
          }
        }

      delay(1);
      keyWait--;
  }
}

//search input for commands
// ASCII 0 = start/stop playback
// ASCII 1-7 = note
// ASCII 8 = change MUSICAL mode
// ASCII 9 = toggle keyboard/tracker

int findKey(char inKey){

// If 1-7 return interval 1-7
  int keys[] = {49,50,51,52,53,54,55};
    for(int i = 0; i < 7; i++){
        if(keys[i] == inKey) return (i+1);
    }

// If ASCII 0 toggle volume
    if(inKey == 48){
      volumeToggle = 1;
    }

// If ASCII 8 change musical mode
      if( inKey == 56) {
        modeselect++;
        Serial.println("modeswitched");
        if (modeselect == 7){
          modeselect = 0;
        }
      }

  return 0;
}
