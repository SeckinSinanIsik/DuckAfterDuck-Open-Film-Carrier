
/* DuckAfterDuck OPEN Film Carrier
   Arduino Nano
   Uses Wire and Pololu VL53L0X library https://github.com/pololu/vl53l0x-arduino
   Seckin Sinan Isik
   Under GNU GENERAL PUBLIC LICENSE Version 3
*/

#include <Wire.h>
#include <VL53L0X.h>

VL53L0X sensor;
#define HIGH_SPEED

///////////////////////////////////
///////////VARIABLES///////////////
///////////////////////////////////
#define motStepPin     8
#define motDirPin      7
#define motMS1Pin      11
#define motMS2Pin      10
#define motMS3Pin      9

int analogPin = A3; 
int ElapsedTime, CurrentTime, StartTime, latchT = 0, Time = 15000, IN=0;
int latchLidar=0, cancelLider=0, countButton1; 
int StartTimeLidar,CurrentTimeLidar,ElapsedTimeLidar;

int latchCalib = 0, ThresholdFilm=35;
int G = A1; // RGB color
int B = A2; // RGB color
int R = A0; // RGB color

const int buttonColor = 2;
const int ledPin =  12;
int ThresholdTOF=45;
int latch = 0;
float counter = 0;


// button inputs
#define buttonRev      4
#define buttonRunRev   13
#define buttonRun      6
#define buttonFrw      5
#define buttonPicture  3

int ColorSelect = 0;
int motSpeed = 200, motSpeedLong = 600, FilmProgression, initialPush = 850, verifiyer=0;
///////////////////////////////////
///////////////SETUP///////////////
///////////////////////////////////
void setup() {
  Serial.begin(9600);
  Wire.begin();
  // assigning buttons
  pinMode(buttonRun, INPUT);
  pinMode(buttonRunRev, INPUT);
  pinMode(buttonFrw, INPUT);
  pinMode(buttonRev, INPUT);
  pinMode(buttonPicture, INPUT);
  // configure initial motor settings
  digitalWrite(motStepPin, LOW); //
  digitalWrite(motDirPin, HIGH); //direction
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(motStepPin, OUTPUT);
  pinMode(motDirPin, OUTPUT);
  pinMode(analogPin, OUTPUT) ;   //Powers the stepper
  analogWrite(analogPin, 255) ;  //Powers the stepper at t=0
  // clours
  pinMode(R, OUTPUT) ;  //RGB color, RED
  pinMode(G, OUTPUT) ;  //RGB color, GREEN
  pinMode(B, OUTPUT) ;  //RGB color, BLUE
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  pinMode(buttonColor, INPUT);
  sensor.setTimeout(500);
  if (!sensor.init())
  {
    Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }

  sensor.startContinuous(100);
}
///////////////////////////////////
////////////////LOOP///////////////
///////////////////////////////////
void loop() {
  if (sensor.timeoutOccurred()) { Serial.print(" TIMEOUT"); }
  //////////////////////////////////////
  //Calibration
  if (latchCalib == 0){
    ThresholdTOF=round(filmCheck(10))-8;
    Serial.print("Initial_Callib: ");
    Serial.println(ThresholdTOF);
    latchCalib=1;
  }
  delay(2);
  if (latchCalib == 1 && sensor.readRangeSingleMillimeters()<ThresholdTOF){
    ThresholdFilm=round(filmCheck(10))+3;
    ThresholdTOF=ThresholdTOF+10; //returning back the value
    Serial.print("Film_Callib: ");
    Serial.println(ThresholdFilm);
    latchCalib=latchCalib+1;
  }
////////////////////////////////////
  // Turn off the stepper after 15 seconds 
  if (analogRead(analogPin) > 1010 && latchT == 0 ) {
    StartTime = millis();
    latchT = 1;
  }
  CurrentTime = millis();
  ElapsedTime = CurrentTime - StartTime;
  if (ElapsedTime > Time && latchT == 1 ) {
    analogWrite(analogPin, 0);
    latchT = 0;
  }
//////////////////////////////////////
  //LIDAR readings are evaluated (3 positive readings in a row is checked)
  if (sensor.readRangeSingleMillimeters() < ThresholdFilm && latch==0 && latchCalib==2) {
    if(verifiyer>=0 && verifiyer<3) {verifiyer=verifiyer+1;Serial.print(verifiyer);} //let the film in
  }
  else {
    if(verifiyer<=3 && verifiyer>0) {verifiyer=verifiyer-1;}
  }
  // If there is 3 positive reading and film is not already accepted in the film carrier
  if (verifiyer==3 && latch==0 ) { 
    delay(300);  
    //film entry sequence 
    Serial.println(initialPush);
    stepper(motStepPin, motSpeedLong, initialPush, 0, 1, 0, 0);
    delay(50);
    if (filmCheck(5) < ThresholdFilm+5) {latch = 1;} //If the film is considered in the film carrier
    stepper(motStepPin, motSpeedLong, 300, 1, 1, 0, 0);
    stepper(motStepPin, motSpeedLong, 300, 0, 1, 0, 0);
    if (filmCheck(5) >= ThresholdFilm+5) {latch=0;} //film is NOT considered in the film carrier
  }
   
  if (counter*FilmProgression>700 && latch==1) { //film out action
    latch=0;
    //film push out
    stepper(motStepPin, motSpeedLong, 400, 0, 1, 0, 0);
    counter=0;
  }

        

//////////////////////////////////////
  // button 1, Toggle between film format for button 5

  if (digitalRead(buttonColor) == LOW) {
    
      ColorSelect = ColorSelect + 1;
      if (ColorSelect == 5) {
        ColorSelect = 0;
      }
      // LED on the Arduino board, blink when toggling
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      latchLidar=0;
      countButton1=0;
    }
//////////////////////////////////////

  
  switch (ColorSelect) {
    case 0:    // 645 format
      setColor(200, 0, 0);  // red
      FilmProgression = 345;
      break;
    case 1:    // square format
      setColor(0, 200, 0); // green
      FilmProgression = 460;
      break;
    case 2:    // 6x7 format
      setColor(0, 0, 200);  // blue
      FilmProgression = 537;
      break;
    case 3:    // 6*9 format
      setColor(200, 0, 200);  // purple
      FilmProgression = 690;
      break;
    case 4:    // 35mm
      setColor(0, 200, 200);  // aqua
      FilmProgression = 268;
      break;
  }

  //////////////////////////////////////
  
  // button 4, MircoStep Forward
  if (digitalRead(buttonFrw) == LOW) {
    delay(50);
    stepper(motStepPin, motSpeed, 32, 0, 1, 1, 1);
  }
  // button 3, MircoStep Reverse
  if (digitalRead(buttonRev) == LOW ) {
    delay(50);
    stepper(motStepPin, motSpeed, 32, 1, 1, 1, 1);
  }
  // button 5, Full Step Forward
  if (digitalRead(buttonRun) == LOW ) {
    if (digitalRead(analogPin) == 0) {
      digitalWrite(analogPin, 255);
    }  
    stepper(motStepPin, motSpeedLong, FilmProgression, 0, 1, 0, 0);
    delay(50);    
    if (latchCalib==2 && latch == 1){ if (filmCheck(5) >= ThresholdTOF -5) {counter = counter + 1; } //count if the film is not under the sensor
    Serial.println(counter);
    }
  }
  // button 2, Take picture
  if (digitalRead(buttonPicture) == LOW) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(400);
  }
}
///////////////////////////////////
////////////////END////////////////
///////////////////////////////////
void setColor(int red, int green, int blue){
  analogWrite(R, red);
  analogWrite(G, green);
  analogWrite(B, blue);
}
long stepper(int stepPin, int wait, long FilmProgression, int dir, int x, int y, int z) {
  int Sign;
   if (dir==0){Sign=1;}
   else {Sign=-1;}
   
  analogWrite(analogPin, 250); //Turn on stepper 
  latchT = 0;                  //reset counting
  digitalWrite(motDirPin, dir); // direction input
  digitalWrite(motMS1Pin, x); //[MS1][MS2][MS3] all 0 for activatin full step
  digitalWrite(motMS2Pin, y);
  digitalWrite(motMS3Pin, z);
  if (latch == 1 && counter==0 && z==1) { //film in, only micro movements are made== assume film adjustment
    initialPush = initialPush + (Sign)*4;
    Serial.println(initialPush);
  }    
  for (int x = 1; x <= FilmProgression; x++) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(wait);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(wait);
    }
  }
int filmCheck(int checkNumber) {
    int array [checkNumber];
    for (int i = 1; i<=checkNumber; i++) {
      array [i]= sensor.readRangeSingleMillimeters();
      delay(10);
    }
     return avg(array, checkNumber);
} 
float avg(int *array, int len) {
  float sum = 0;  
  for (int i=1; i<=len; i++) { sum= sum + array [i];}
  
  return  sum/len;  
}
