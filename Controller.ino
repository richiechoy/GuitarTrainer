/*
Richie Choy - Spring 2014
Georgia Tech InVenture Prize Competition - software implementation for Finals
Implements the embedded devices in the Guitar Trainer

TIME STAMP: 3/22 - includes:
-guitar and bass songs populated
*/

#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <glcd.h>
#include <fonts/allFonts.h>

// string pins - bit0 is the high E string
const int string1 = 23;
const int string2 = 24;
const int string3 = 25;
const int string4 = 26;
const int string5 = 27;
const int string6 = 28;

// new fret BJT enable pins
const int fretEN0 = 2;
const int fretEN1 = 3;
const int fretEN2 = 4;
const int fretEN3 = 5;
const int fretEN4 = 6;
const int fretEN5 = 7;
const int fretEN6 = 8;
const int fretEN7 = 9;
const int fretEN8 = 10;
const int fretEN9 = 11;
const int fretEN10 = 12;
const int fretEN11 = 13;
const int fretEN12 = 22;

// metronome
const int speakerPin = 50; 
unsigned long Timer = 0;
int metronomeState = 1;
unsigned long metronomeDelay = 0;
int metronomeEnable = 0;

// Time parameters
int tempo = 60; // init at 60 bpm
unsigned long timeHolder = 0; // for time tracking
unsigned long totalBeats = 0; // track total quarter notes elapsed
unsigned long lastBeat = 0; // global for duration counting
int playFlag = 0;
int delayFlag = 1;
int delayCount = 0;

// button states
int tUpState1 = 0;
int tUpState2 = 0;
int tDownState1 = 0;
int tDownState2 = 0;
int mEnableState1 = 0;
int mEnableState2 = 0;
int startState1 = 0;
int startState2 = 0;
int nextState1 = 0;
int nextState2 = 0;
int modeState1 = 0;
int modeState2 = 0;
int resetState1 = 0;
int resetState2 = 0;
int keyState1 = 0;
int keyState2 = 0;

// User Interface Buttons
const int upButtonPin = 48;  
const int downButtonPin = 45;  
const int startPin = 42;
const int nextPin = 43; 
const int modePin = 46;
const int resetPin = 49;
const int keyPin = 44; 
const int mEnablePin = 47;  

// global variables for Music Training 
const char* noteArray[]={"A", "A#", "B", "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#"};
const int numProgressions = 7;
const int numScales = 4;
int myKey = 3; // index in noteArray, inititialized as "C"
int beatCount = 0;
int measureCount = 1;
int randVal = 0;
//chord vars
int chordBuffer[4];
int one = 0;
int four = 0;
int five = 0;
int iMode = 1;
//scale vars
int scaleBuffer[8];
int scaleIndex = 0;

// song globals
int songPick = 0;
int songIndex = 0;
const int guitarLengthArray[] = {8, 16};
const int bassLengthArray[] = {16, 8};
const int numGuitarSongs = 2;
const int numBassSongs = 2;

// holding down tempo up down buttons
unsigned long buttonHeldDown = 0;
boolean trackUpFlag = true;
boolean playFirst = true;

void setup()
{
    // initialize the power line for each string
    pinMode(string1, OUTPUT); 
    pinMode(string2, OUTPUT); 
    pinMode(string3, OUTPUT); 
    pinMode(string4, OUTPUT); 
    pinMode(string5, OUTPUT); 
    pinMode(string6, OUTPUT); 
    
    // initialize the BJT enables for each fret
    pinMode(fretEN0, OUTPUT); 
    pinMode(fretEN1, OUTPUT); 
    pinMode(fretEN2, OUTPUT); 
    pinMode(fretEN3, OUTPUT); 
    pinMode(fretEN4, OUTPUT); 
    pinMode(fretEN5, OUTPUT); 
    pinMode(fretEN6, OUTPUT); 
    pinMode(fretEN7, OUTPUT); 
    pinMode(fretEN8, OUTPUT); 
    pinMode(fretEN9, OUTPUT);
    pinMode(fretEN10, OUTPUT); 
    pinMode(fretEN11, OUTPUT); 
    pinMode(fretEN12, OUTPUT);  
    
    // initialize button pins
    pinMode(upButtonPin, INPUT); // 
    pinMode(downButtonPin, INPUT);
    pinMode(startPin, INPUT);
    pinMode(nextPin, INPUT);
    pinMode(modePin, INPUT);
    pinMode(resetPin, INPUT);
    pinMode(keyPin, INPUT);
    pinMode(mEnablePin, INPUT);
    digitalWrite(upButtonPin, HIGH);
    digitalWrite(downButtonPin, HIGH);
    digitalWrite(startPin, HIGH);
    digitalWrite(nextPin, HIGH);
    digitalWrite(modePin, HIGH);
    digitalWrite(resetPin, HIGH);
    digitalWrite(keyPin, HIGH);
    digitalWrite(mEnablePin, HIGH);
    
    // initialize serial bus at 9600 baud rate
    Serial.begin(9600);
    // init LCD
    GLCD.Init(NON_INVERTED);
    
    makeProgression();
    makeScale();
    LCDprintLabels();
}


void loop()
{
    buttonListener();
    int tick = getTick();
    
    if(playFlag == 1 && delayFlag == 1)
    {
        startDelay(tick);
    }

    // maybe: countMeasures needs tick == 1 and the rest need to be on in between ticks too
    if(playFlag == 1 && delayFlag == 0)
    {
        if(tick == 1)
        {
            if(iMode == 2 || iMode ==3)
            {
                countSong();
            }  
            else
            {
                countMeasures(); // must do this before calling playProgression or playScale
            }
            
            if(metronomeEnable == 1)
            {
                metronome(); // may cause timing problems....
            }
        }
        
        if(iMode == 1)
        {
            playProgression();
            followChords();
        }
        else if (iMode == 0)
        {
            playScale();
        }
        else if(iMode == 2)
        {
            playGuitarSong();    
        }
        else if(iMode == 3)
        {
            playBassSong();    
        }
    }
}

void playNote(int stringNum, int fretNum)
{   
    int enableArray[13] = {fretEN0, fretEN1, fretEN2, fretEN3, fretEN4, fretEN5,
        fretEN6, fretEN7, fretEN8, fretEN9, fretEN10, fretEN11, fretEN12};
    for(int i = 0; i < 13; i++)
    {
        digitalWrite(enableArray[i], LOW);
    }
    int stringArray[6] = {string1, string2, string3, string4, string5, string6};
    for(int i = 0; i < 6; i++)
    {
        digitalWrite(stringArray[i], LOW);
    } 
    
    if(stringNum > 0)
    {
        // now turn on the correct string and bjt enable
        digitalWrite(stringArray[stringNum-1], HIGH);
        digitalWrite(enableArray[fretNum], HIGH);
        // and turn them off
        digitalWrite(enableArray[fretNum], LOW);
        digitalWrite(stringArray[stringNum-1], LOW);
    }
    /*
    Serial.print("String: ");
    Serial.print(stringNum);
    Serial.print("||| Fret: ");
    Serial.println(fretNum);
    delay(500);
    */
}

void playProgression()
{    
    int chordIndex = 0;
    
    if(measureCount>0) // protect bounds
    {
        chordIndex = chordBuffer[measureCount-1];
    }
    playChord(noteArray[chordIndex]); 
}

void makeProgression()
{
    for(int i = 0; i < 12; i++)
    {
        if(getKey(myKey) == noteArray[i])
        {
            one = i;
        }      
    }
    four = (one + 5)%12;
    five = (one + 7)%12;

    switch(randVal) // could be local var...
    {
        case 0: //1-4-5-5
            chordBuffer[0] = one;
            chordBuffer[1] = four;
            chordBuffer[2] = five;
            chordBuffer[3] = five;
            break;
        case 1: // 1-4-5-1
            chordBuffer[0] = one;
            chordBuffer[1] = four;
            chordBuffer[2] = five;
            chordBuffer[3] = one;
            break;
        case 2: // 1-4-1-5
            chordBuffer[0] = one;
            chordBuffer[1] = four;
            chordBuffer[2] = one;
            chordBuffer[3] = five;
            break;
        case 3: // 1-4-5-4
            chordBuffer[0] = one;
            chordBuffer[1] = four;
            chordBuffer[2] = five;
            chordBuffer[3] = four;
            break;
        case 4: // 1-4-5-5
            chordBuffer[0] = one;
            chordBuffer[1] = four;
            chordBuffer[2] = five;
            chordBuffer[3] = five;
            break;
        case 5: // 1-5-1-5
            chordBuffer[0] = one;
            chordBuffer[1] = five;
            chordBuffer[2] = one;
            chordBuffer[3] = five;
            break;
        case 6: // 1-1-1-1
            chordBuffer[0] = one;
            chordBuffer[1] = one;
            chordBuffer[2] = one;
            chordBuffer[3] = one;
            break;                    
        default:
            chordBuffer[0] = one;
            chordBuffer[1] = one;
            chordBuffer[2] = one;
            chordBuffer[3] = one;
            break;
    }    
}

void playScale()
{
    if(measureCount < 3)
    {
        scaleIndex = beatCount-1+((measureCount-1)*4);
    }
    else
    {
        scaleIndex = 8 - beatCount - (measureCount-3)*4;
    }
    if(scaleIndex < 0) // double check bounds
    {
        scaleIndex = 0;
    }
    playNote( addressToStringNum(scaleBuffer[scaleIndex]), addressToFretNum(scaleBuffer[scaleIndex]) );
}

void moveableScale(int stringNum, int fretNum)
{   // limits are: stringNum > 2 && fretnum > 0; fretnum < 9
    if(stringNum == 4) // string - 2 gets shifted on B string
    {
        scaleBuffer[0] = getAddress(stringNum, fretNum);
        scaleBuffer[1] = getAddress(stringNum, fretNum+2);
        scaleBuffer[2] = getAddress(stringNum-1, fretNum-1); // fretNum-1+j, j is 0 if stringNum is past the B string at that time
        scaleBuffer[3] = getAddress(stringNum-1, fretNum);
        scaleBuffer[4] = getAddress(stringNum-1, fretNum+2);
        scaleBuffer[5] = getAddress(stringNum-2, fretNum);
        scaleBuffer[6] = getAddress(stringNum-2, fretNum+2);
        scaleBuffer[7] = getAddress(stringNum-2, fretNum+3); // root again
    }
    else if(stringNum == 3) // both lower str get shifted up a fret
    {
        scaleBuffer[0] = getAddress(stringNum, fretNum);
        scaleBuffer[1] = getAddress(stringNum, fretNum+2);
        scaleBuffer[2] = getAddress(stringNum-1, fretNum); // fretNum-1+j, j is 0 if stringNum is past the B string at that time
        scaleBuffer[3] = getAddress(stringNum-1, fretNum+1);
        scaleBuffer[4] = getAddress(stringNum-1, fretNum+3);
        scaleBuffer[5] = getAddress(stringNum-2, fretNum);
        scaleBuffer[6] = getAddress(stringNum-2, fretNum+2);
        scaleBuffer[7] = getAddress(stringNum-2, fretNum+3); // root again
    }
    else // case of strNum = 6,5: otherwise just do the normal stuff - 
    {
        scaleBuffer[0] = getAddress(stringNum, fretNum);
        scaleBuffer[1] = getAddress(stringNum, fretNum+2);
        scaleBuffer[2] = getAddress(stringNum-1, fretNum-1); // fretNum-1+j, j is 0 if stringNum is past the B string at that time
        scaleBuffer[3] = getAddress(stringNum-1, fretNum);
        scaleBuffer[4] = getAddress(stringNum-1, fretNum+2);
        scaleBuffer[5] = getAddress(stringNum-2, fretNum-1);
        scaleBuffer[6] = getAddress(stringNum-2, fretNum+1);
        scaleBuffer[7] = getAddress(stringNum-2, fretNum+2); // root again  
    }
}

int getAddress(int stringNum, int fretNum)
{
    // check bounds: strings[0,6] & frets[0, 
    if(stringNum < 1 || stringNum > 6)
    {
       stringNum = 1; 
    }
    if(fretNum < 0 || fretNum > 12)
    {
       fretNum = 0; 
    }
    return fretNum + (stringNum - 1)*13;
}

int addressToStringNum(int address)
{
     return floor( ((address)/13 ) );
}

int addressToFretNum(int address)
{
     return address%13;
}

void metronome()
{
      tone(speakerPin, 2500 , 50); // play
}

int getTick()
{
    float beatDuration = 60.0 / (float) tempo * 1000; // each quarter note size
    //Serial.println(beatDuration);
    unsigned long now = millis();
    if(now - timeHolder >  beatDuration)
    {
        timeHolder = now;
        totalBeats++;
        return 1;
    }
  return 0;
}

String getKey(int keyIndex)
{
    return noteArray[keyIndex];
}

void countMeasures()  // keeps time, incrementing globals
{
    if(beatCount == 4)
    {
        beatCount = 1;  // reset beat count
        if(measureCount == 4)
        {
            measureCount = 1;  // reset measure count
        }
        else
        {
            measureCount++;
        }
    }
    else 
    {
        beatCount++; 
    } 
    
    Serial.print("beat: ");
    Serial.println(beatCount);
    Serial.print("measure: ");
    Serial.println(measureCount); 
}

void startDelay(int tick)
{
    
    if(delayCount == 0)
    {
        metronome(); 
        timeHolder = millis(); // if this is called before a tick, it will beep and the next tick occurs after the fixed beat
        delayCount++;
        //showDelay();
    }
    else if(tick == 1)
    {
        if(delayCount < 4)
        {
            metronome();   
            delayCount++; 
            //showDelay();
        }
        else
        {
            delayFlag = 0; // stop delaying
            delayCount = 0;  
            if(iMode == 1)
            {
                //LCDprintLabels(); // or show scale
            }
            else if(iMode == 0)
            {
                //LCDprintLabels();  
            }
        }    
    }    
}

int nextSet()
{
    if(iMode == 1)
    {
        newRand(numProgressions); // input max number of rand progressions
    }
    else if (iMode == 0)
    {
        newRand(numScales);
    }
    else if(iMode == 2)
    {
        if(songPick == numGuitarSongs - 1) // if there are 3 songs, we want to go back to index 0 at pick #2
        {
            songPick = 0;
            tempo = 70;
        } 
        else
        { 
            songPick++;
        }
    }
    else if(iMode == 3)
    {
        if(songPick == numBassSongs - 1) // if there are 3 songs, we want to go back to index 0 at pick #2
        {
            songPick = 0;
            tempo = 290;
        } 
        else
        { 
            songPick++;
        }
    }    
    reset();
    LCDwriteSongs();
}

void nextKey()
{
    if(iMode == 0 || iMode == 1)
    {
        myKey = (myKey+1)%12;
        reset();  
    }
}

void reset()
{
    beatCount = 0;
    measureCount = 1;  
    playFlag = 0;
    delayFlag = 1;
    delayCount = 0;
    songIndex = 0;
    playFirst = true;
    
    clearAllLED(); // should address lingering lights on reset
    
    if(iMode == 1)
    {
        makeProgression();
    }
    else if(iMode == 0)
    {
        makeScale();
    }
    LCDprintLabels();
}

void changeMode()
{
    if(iMode == 3)
    {
        iMode = 0;  
    }
    else
    {
       iMode++;
    }
  
    if(iMode == 1)
    {
        makeScale();
    }  
    else if(iMode == 0)
    {
        makeProgression();       
    }  
    else if(iMode == 2)
    {
        songPick = 0;
        tempo = 70;
    }
    else if(iMode == 3)
    {
        songPick = 0;
        tempo = 290;
    }
    reset();
    LCDwriteSongs();
    Serial.println("iMode:");
    Serial.println(iMode);
}

void newRand(int maxVal)
{
    int newRand = random(maxVal);
    while(newRand == randVal)
    {
        newRand = random(maxVal);
    }
    randVal = newRand;
}

void buttonListener() 
{
    // Listen to Up Button
    tUpState1 = digitalRead(upButtonPin);    
    if( tUpState1 != tUpState2)  // if state change
    {
        if(tUpState1 == LOW)
        {
            if(tempo < 400)
            {
                tempo += 5;
                LCDprintLabels();
            }
        }
    }
    tUpState2 = tUpState1;
    
    // Listen to Down Button
    tDownState1 = digitalRead(downButtonPin);    
    if( tDownState1 != tDownState2)  // if state change
    {
        if(tDownState1 == LOW)
        {
            if(tempo > 1)
            {
                tempo -= 5;
                LCDprintLabels();
            }
        }
    }
    tDownState2 = tDownState1;
    
    
    // Listen to Metronome Enable Button
    mEnableState1 = digitalRead(mEnablePin);    
    if(mEnableState1 != mEnableState2)  // if state change
    {
        if(mEnableState1 == LOW)
        {
            if(metronomeEnable == 1)
            {
                metronomeEnable = 0;
                LCDprintLabels();
            }  
            else 
            {
                metronomeEnable = 1; 
                LCDprintLabels();
            }
        }
    }
    mEnableState2 = mEnableState1;
    
    
    // Listen to Start/Pause Button
    startState1 = digitalRead(startPin); 
    if(startState1 != startState2)  // if state change
    {
        if(startState1 == LOW)
        {
            if(playFlag == 1)
            {
                playFlag = 0;
                delayFlag = 1;
                delayCount = 0;
                LCDprintLabels();
            }  
            else 
            {
                playFlag = 1;
               if(delayFlag == 1)
              {
                  delayCount = 0; 
                  //timeHolder = millis();
              } 
              printPlay();
            }
            
        }
    }
    startState2 = startState1; 
    
    // Listen to Next Button
    nextState1 = digitalRead(nextPin);    
    if(nextState1 != nextState2)  // if state change
    {
        if(nextState1 == LOW)
        {
            nextSet();
        }
    }
    nextState2 = nextState1;   
    
    // Listen to Mode Button
    modeState1 = digitalRead(modePin);    
    if(modeState1 != modeState2)  // if state change
    {
        if(modeState1 == LOW)
        {
            changeMode();
            LCDprintLabels();
        }
    }
    modeState2 = modeState1;  
    
    // Listen to Mode Button
    resetState1 = digitalRead(resetPin);    
    if(resetState1 != resetState2)  // if state change
    {
        if(resetState1 == LOW)
        {
            reset();
        }
    }
    resetState2 = resetState1; 
    
    // Listen to Key Button
    keyState1 = digitalRead(keyPin);    
    if(keyState1 != keyState2)  // if state change
    {
        if(keyState1 == LOW)
        {
            nextKey();
            LCDprintLabels();
        }
    }
    keyState2 = keyState1; 
    
    if(tUpState1 == LOW && tDownState1 == LOW)
    {
        debugFlashAll();  
    }
}

void playChord(String chord)
{   
    if(chord == "A")
    {
        playNote(5, 0);
        playNote(4, 2);
        playNote(3, 2);
        playNote(2, 2);
        playNote(1, 0);
    }
    else if(chord == "A#")
    {
        playNote(5, 1);
        playNote(4, 3);
        playNote(3, 3);
        playNote(2, 3);
        //playNote(1, 1);
    }
    else if(chord == "B")
    {
        playNote(5, 2);
        playNote(4, 4);
        playNote(3, 4);
        playNote(2, 4);
        //playNote(1, 2);
    }    
    else if(chord == "C")
    {
        playNote(5, 3);
        playNote(4, 2);
        playNote(3, 0);
        playNote(2, 1);
        playNote(1, 0);
    }     
    else if(chord == "C#")
    {
        playNote(5, 4);
        playNote(4, 3);
        playNote(3, 1);
        playNote(2, 2);
        playNote(1, 1);
    }     
    else if(chord == "D")
    {
        playNote(4, 0);
        playNote(3, 2);
        playNote(2, 3);
        playNote(1, 2);
    }
    else if(chord == "D#")
    {
        playNote(5, 6);
        playNote(4, 5);
        playNote(3, 3);
        playNote(2, 4);
        playNote(1, 3);
    }    
    else if(chord == "E")
    {
        playNote(6, 0);
        playNote(5, 2);
        playNote(4, 2);
        playNote(3, 1);
        playNote(2, 0);
        playNote(1, 0);
    }    
    else if(chord == "F")
    {
        playNote(6, 1);
        playNote(5, 3);
        playNote(4, 3);
        playNote(3, 2);
        playNote(2, 1);
        playNote(1, 1);
    }    
    else if(chord == "F#")
    {
        playNote(6, 2);
        playNote(5, 4);
        playNote(4, 4);
        playNote(3, 3);
        playNote(2, 2);
        playNote(1, 2);
    }     
    else if(chord == "G")
    {
        playNote(6, 3);
        playNote(5, 2);
        playNote(4, 0);
        playNote(3, 0);
        playNote(2, 0);
        playNote(1, 3);
    }
    else if(chord == "G#")
    {
        playNote(6, 4);
        playNote(5, 6);
        playNote(4, 6);
        playNote(3, 5);
        playNote(2, 4);
        playNote(1, 4);
    }    
    else
    {
        //clearPins();
    }
}


void makeScale() // this input is the root's position
{
      switch(myKey)
      {
          case 0: // A
              if(randVal == 0) // open pos
              {
                  scaleBuffer[0] = getAddress(5, 0);
                  scaleBuffer[1] = getAddress(5, 2);
                  scaleBuffer[2] = getAddress(5, 4);
                  scaleBuffer[3] = getAddress(4, 0);
                  scaleBuffer[4] = getAddress(4, 2);
                  scaleBuffer[5] = getAddress(4, 4);
                  scaleBuffer[6] = getAddress(3, 1);
                  scaleBuffer[7] = getAddress(3, 2);
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 2); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(6, 5); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(4, 7); 
              }
          break;

          case 1: // A#
              if(randVal == 0) // open pos
              {
                  moveableScale(5, 1); // works with open
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 3); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(6, 6); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(4, 8); 
              }
          break;
                    
          case 2: // B
              if(randVal == 0) // open pos
              {
                  moveableScale(5, 2); // works with open
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 4); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(6, 7); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(4, 9); // close - but ok
              }
          break;
        
          case 3: // C
              if(randVal == 0) // open pos
              {
                  scaleBuffer[0] = getAddress(5, 3);
                  scaleBuffer[1] = getAddress(4, 0);
                  scaleBuffer[2] = getAddress(4, 2);
                  scaleBuffer[3] = getAddress(4, 3);
                  scaleBuffer[4] = getAddress(3, 0);
                  scaleBuffer[5] = getAddress(3, 2);
                  scaleBuffer[6] = getAddress(2, 0);
                  scaleBuffer[7] = getAddress(2, 1);
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 5); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(6, 8); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 3);
              }
          break;

          case 4: // C#
              if(randVal == 0) // open pos
              {
                  scaleBuffer[0] = getAddress(5, 4);
                  scaleBuffer[1] = getAddress(4, 1);
                  scaleBuffer[2] = getAddress(4, 3);
                  scaleBuffer[3] = getAddress(4, 4);
                  scaleBuffer[4] = getAddress(3, 1);
                  scaleBuffer[5] = getAddress(3, 3);
                  scaleBuffer[6] = getAddress(2, 1);
                  scaleBuffer[7] = getAddress(2, 2); 
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 6); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(6, 9); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 4); 
              }
          break;
          
          case 5: // D
              if(randVal == 0) // open pos
              {
                  scaleBuffer[0] = getAddress(4, 0);
                  scaleBuffer[1] = getAddress(4, 2);
                  scaleBuffer[2] = getAddress(4, 4);
                  scaleBuffer[3] = getAddress(3, 0);
                  scaleBuffer[4] = getAddress(3, 2);
                  scaleBuffer[5] = getAddress(2, 0);
                  scaleBuffer[6] = getAddress(2, 2);
                  scaleBuffer[7] = getAddress(2, 3);
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 7); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(6, 10); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 5); 
              }
          break;          

          case 6: // D#
              if(randVal == 0) // open pos
              {
                  moveableScale(4, 1); // works with open
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 8); 
              }
              else if(randVal == 2)
              {        
                  scaleBuffer[0] = getAddress(5, 6);
                  scaleBuffer[1] = getAddress(5, 8);
                  scaleBuffer[2] = getAddress(5, 10);
                  scaleBuffer[3] = getAddress(4, 6);
                  scaleBuffer[4] = getAddress(4, 8);
                  scaleBuffer[5] = getAddress(4, 10);
                  scaleBuffer[6] = getAddress(3, 7);
                  scaleBuffer[7] = getAddress(3, 8);
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 6); 
              }
          break;
          
          case 7: // E
              if(randVal == 0) // open pos
              {
                  scaleBuffer[0] = getAddress(6, 0);
                  scaleBuffer[1] = getAddress(6, 2);
                  scaleBuffer[2] = getAddress(6, 4);
                  scaleBuffer[3] = getAddress(5, 0);
                  scaleBuffer[4] = getAddress(5, 2);
                  scaleBuffer[5] = getAddress(5, 4);
                  scaleBuffer[6] = getAddress(4, 1);
                  scaleBuffer[7] = getAddress(4, 2);
              }
              else if(randVal == 1)
              {        
                  moveableScale(3, 9); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(4, 2); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 7); 
              }
          break;           

          case 8: // F
              if(randVal == 0) // open pos
              {
                  moveableScale(6, 1); // works with open
              }
              else if(randVal == 1)
              {        
                  scaleBuffer[0] = getAddress(4, 3);
                  scaleBuffer[1] = getAddress(4, 5);
                  scaleBuffer[2] = getAddress(4, 7);
                  scaleBuffer[3] = getAddress(3, 3);
                  scaleBuffer[4] = getAddress(3, 5);
                  scaleBuffer[5] = getAddress(3, 7);
                  scaleBuffer[6] = getAddress(2, 5);
                  scaleBuffer[7] = getAddress(2, 6);
              }
              else if(randVal == 2)
              {        
                  moveableScale(4, 3); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 8); 
              }
          break;

          case 9: // F#
              if(randVal == 0) // open pos
              {
                  moveableScale(6, 2); // works with open
              }
              else if(randVal == 1)
              {        
                  scaleBuffer[0] = getAddress(4, 4);
                  scaleBuffer[1] = getAddress(4, 6);
                  scaleBuffer[2] = getAddress(4, 8);
                  scaleBuffer[3] = getAddress(3, 4);
                  scaleBuffer[4] = getAddress(3, 6);
                  scaleBuffer[5] = getAddress(3, 8);
                  scaleBuffer[6] = getAddress(2, 6);
                  scaleBuffer[7] = getAddress(2, 7);
              }
              else if(randVal == 2)
              {        
                  moveableScale(4, 4); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(5, 9); 
              }
          break;
          
          case 10: // G
              if(randVal == 0) // open pos
              {
                  scaleBuffer[0] = getAddress(6, 3);
                  scaleBuffer[1] = getAddress(5, 0);
                  scaleBuffer[2] = getAddress(5, 2);
                  scaleBuffer[3] = getAddress(5, 3);
                  scaleBuffer[4] = getAddress(4, 0);
                  scaleBuffer[5] = getAddress(4, 2);
                  scaleBuffer[6] = getAddress(4, 4);
                  scaleBuffer[7] = getAddress(3, 0);
              }
              else if(randVal == 1)
              {        
                  moveableScale(6, 3); 
              }
              else if(randVal == 2)
              {        
                  moveableScale(5, 10); 
              }
              else if(randVal == 3)
              {        
                  moveableScale(4, 5); 
              }
          break;          

          case 11: // G#
              if(randVal == 0) // open pos
              {
                  moveableScale(3, 1); // works with open
              }
              else if(randVal == 1)
              {        
                  moveableScale(6, 4); 
              }
              else if(randVal == 2)
              {        
                  scaleBuffer[0] = getAddress(4, 6);
                  scaleBuffer[1] = getAddress(4, 8);
                  scaleBuffer[2] = getAddress(4, 10);
                  scaleBuffer[3] = getAddress(3, 6);
                  scaleBuffer[4] = getAddress(3, 8);
                  scaleBuffer[5] = getAddress(3, 10);
                  scaleBuffer[6] = getAddress(2, 8);
                  scaleBuffer[7] = getAddress(2, 9);
              }
              else if(randVal == 3)
              {        
                  moveableScale(4, 6); 
              }
          break;        
      }
}


void LCDprintLabels()
{
  GLCD.ClearScreen();  
  // header
  GLCD.SelectFont(Arial_bold_14);
  GLCD.CursorTo(1, 0);
  GLCD.print("enLighten");
  GLCD.CursorTo(1, 1);
  GLCD.print("Music Trainer");
  GLCD.DrawLine(2, 27, 125, 27, BLACK); 
  GLCD.DrawLine(2, 45, 125, 45, BLACK); 

  // details - key, mode
  GLCD.SelectFont(System5x7);
  // tempo
  GLCD.CursorTo(0, 6);
  GLCD.print("Tempo:");
  // metronome
  GLCD.CursorTo(11, 6);
  GLCD.print("Nome:");
  // key
  GLCD.CursorTo(0, 7);
  GLCD.print("Key:");

  // mode
  GLCD.CursorTo(9, 7);
  GLCD.print("Mode:");

  LCDwriteTempo();
  LCDwriteKey();
  LCDwriteMode();
  LCDwriteMenable();
  if(iMode == 1)
  {
      LCDwriteChords();
  }
  
}

void LCDwriteTempo()
{
  GLCD.CursorTo(7, 6);
  GLCD.print(tempo);  
}

void LCDwriteKey()
{
  GLCD.CursorTo(5, 7);
  GLCD.print(noteArray[myKey]);
}

void LCDwriteMenable()
{
  GLCD.CursorTo(17, 6);
  if(metronomeEnable == 1)
  {
    GLCD.print("ON");
  }
  else
  {
    GLCD.print("OFF");
  }
}

void LCDwriteMode()
{
  GLCD.CursorTo(15, 7);
  if(iMode == 1)
  {
    GLCD.print("CHORD");
  }
  else if(iMode == 0)
  {
    GLCD.print("SCALE");    
  }
  else if(iMode == 2)
  {
    GLCD.print("GUITAR");  
  }
  else if(iMode == 3)
  {
    GLCD.print("BASS");  
  } 
}

void LCDwriteChords()
{
  // notes
  GLCD.CursorTo(0, 4);
  GLCD.print(noteArray[chordBuffer[0]]);
  GLCD.CursorTo(3, 4);
  GLCD.print(noteArray[chordBuffer[1]]);
  GLCD.CursorTo(6, 4);
  GLCD.print(noteArray[chordBuffer[2]]);
  GLCD.CursorTo(9, 4);
  GLCD.print(noteArray[chordBuffer[3]]);
  GLCD.CursorTo(12, 4);
  GLCD.print("Play:");
  
}

void LCDwriteSongs()
{
    GLCD.CursorTo(0, 4);
    if(iMode == 2 || iMode == 3)
    {
        if(songPick == 0)
        {
            GLCD.print("SONG #1");
        }
        else if(songPick == 1)
        {
            GLCD.print("SONG #2");
        }
    }
    
}

void clearAllLED()
{
  int enableArray[13] = {fretEN0, fretEN1, fretEN2, fretEN3, fretEN4, fretEN5,
        fretEN6, fretEN7, fretEN8, fretEN9, fretEN10, fretEN11, fretEN12};

    // disable all BJTs
    for(int i = 0; i < 13; i++)
    {
        digitalWrite(enableArray[i], LOW);
    }
    
    // turn off all string s
    int stringArray[6] = {string1, string2, string3, string4, string5, string6};
    for(int i = 0; i < 6; i++)
    {
        digitalWrite(stringArray[i], LOW);
    }   
}

void followChords()
{
    // current spot
    if(measureCount > 0)
    {
      GLCD.CursorTo(18, 4);
      GLCD.print(noteArray[chordBuffer[measureCount-1]]);
    } 
}

void debugFlashAll()
{
   int enableArray[13] = {fretEN0, fretEN1, fretEN2, fretEN3, fretEN4, fretEN5,
        fretEN6, fretEN7, fretEN8, fretEN9, fretEN10, fretEN11, fretEN12};

    // disable all BJTs
    for(int i = 0; i < 13; i++)
    {
        digitalWrite(enableArray[i], HIGH);
    }
    // turn off all string s
    int stringArray[6] = {string1, string2, string3, string4, string5, string6};
    for(int i = 0; i < 6; i++)
    {
        digitalWrite(stringArray[i], HIGH);
    }   
    Serial.print("Flashing All");
}

void countSong()
{
    // put this in a guitar or bass if statement
    int lastBeat = 0;
    if(iMode == 2)
    {
        lastBeat = guitarLengthArray[songPick];
    }
    else if(iMode == 3)
    {
        lastBeat = bassLengthArray[songPick];
    }
    
    if(songIndex == lastBeat)
    {
        songIndex = 0;
    }
    else
    {
        if(playFirst)
        {
            songIndex = 0; 
            playFirst = false;
        }
        else
        {
            songIndex++;
        }
    } 
}

void playGuitarSong()
{
    switch(songPick)
    {
         case 0:
            //Serial.print("GSong 0 + ");
            guitarSong1();
            break;
         case 1:
            //Serial.print("GSong 1 + ");  
            guitarSong2();
            break;
          
    }
  
}

void guitarSong1() // bpm =
{
    switch(songIndex) 
    {
        case 0: // measure 1
            playNote(4,4);
            playNote(5,2);
            break;
        case 1: 
            playNote(4,7);
            playNote(5,5);
            break;
        case 2: 
            playNote(4,7);
            playNote(5,5);
            break;
        case 3: 
            playNote(4,9);
            playNote(5,7);
            break;
        case 4: // measure 2
            playNote(4,12);
            playNote(5,10);
            break;
        case 5:
            playNote(4,12);
            playNote(5,10);
            break;
        case 6: 
            playNote(4,7);
            playNote(5,5);
            break;
        case 7: 
            playNote(4,9);
            playNote(5,7);
            break;         
    }  
}

void guitarSong2()
{
    switch(songIndex)
    {   // eigth notes
        case 0: // measure 1
            playNote(5,2);
            break;
        case 1: 
            playNote(5,2);
            break;
        case 2: //
            playNote(5,2);
            break;
        case 3: 
            clearAllLED();
            break;
        case 4: //
            clearAllLED();
            break;  
        case 5: 
            playNote(5,2);
            break; 
        case 6: //
            playNote(5,4);
            break; 
        case 7: 
            playNote(5,5);
            break; 
        case 8: // measure 2
            clearAllLED();
            break; 
        case 9: 
            clearAllLED();
            break; 
        case 10: //
            clearAllLED();
            break; 
        case 11: 
            playNote(5,5);
            break;    
        case 12: //
            playNote(5,5);
            break; 
        case 13: 
            playNote(5,4);
            break;  
        case 14: //
            playNote(5,4);
            break;  
        case 15: 
            playNote(5,2);
            break;    
    }    
}


void playBassSong()
{
    switch(songPick)
    {
         case 0:
            //Serial.print("BSong 0 + ");
            bassSong1();
            break;
         case 1:
            //Serial.print("BSong 1 + ");   
            bassSong2();
            break;
          
    }
  
}

void bassSong1()
{
    switch(songIndex)
    {
        case 0: // measure 1
            playNote(5,5);
            break;
        case 1: 
            playNote(5,5);
            break;
        case 2: //
            playNote(5,3);
            break;
        case 3: 
            playNote(5,2);
            break;
        case 4: //
            playNote(5,0);
            break;
        case 5: 
            clearAllLED();
            break;       
        case 6: // 
            clearAllLED();
            break;
        case 7: 
            playNote(6,0);
            break;
        case 8: // measure 2
            playNote(6,3);
            break;
        case 9: 
            clearAllLED();
            break;
        case 10: // 2
            clearAllLED();
            break;
        case 11: 
            playNote(5,2);
            break;         
        case 12: // 3
            playNote(5,0);
            break;
        case 13: 
            clearAllLED();
            break;  
        case 14: // 4
            clearAllLED();
            break;
        case 15: 
            clearAllLED();
            break;        
    }  
}

void bassSong2()
{
    switch(songIndex)
    {
        case 0: // measure 1
            playNote(6,0);
            break;
        case 1: 
            playNote(6,0);
            break;
        case 2: 
            playNote(6,0);
            break;
        case 3: 
            clearAllLED();
            break;
        case 4: // measure 2
            playNote(6,0);
            break;
        case 5: 
            playNote(6,3);
            break;
        case 6: 
            playNote(6,5);
            break;
        case 7: 
            clearAllLED();
            break;   
    }  
}

void printPlay()
{
    if(playFlag == 1)
    {
        GLCD.CursorTo(9, 4);
        GLCD.print("PLAY!!!!!!!");
    }
}

