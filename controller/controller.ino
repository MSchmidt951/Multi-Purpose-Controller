#include <RF24.h>
#include <FastLED.h>

RF24 radio(2, 30);
byte addresses[][6] = {"C","D"};

CRGB led[1];
const int LEDbrightness = 45;
bool aborted = false;

//Input pins
const int ABORT = 14;
const int Standby = 13;
const int lightBtn = 12;
const int lightSwitch = 11;
const int switches[3] = {4, 3, 0}; //L, M, R
const int triggers[2] = {10, 1}; //L, R

const int dirBtns[4] = {15, 21, 20, 22}; //L, R, U, D
const int btns[2] = {23, 29}; //L, R
const int pot = 31;

const int StickL[3] = {27, 28, 19}; //x, y, JstickBtnL
const int StickR[3] = {25, 24, 26}; //r, z, JstickBtnR

//Data bytes
byte btnByte; //0-3:directional buttons, 4-5:misc buttons, 6-7:triggers
byte miscByte; //0:abort, 1:standby, 2:light, 3-5:switches, 6-7:JstickBtns
char sendData[7]; //[x, y, r, z, pot, btnByte, miscByte]

//Functions

void setLED(int r, int g, int b){
  fill_solid(led, 1, CRGB(r, g, b));
  FastLED.show();
}

int toInt(float f){
  return int(f + 0.5);
}


void setup(){
  //Set up controls
  pinMode(ABORT, INPUT_PULLUP);
  pinMode(Standby, INPUT_PULLUP);
  pinMode(lightBtn, INPUT_PULLUP);
  pinMode(lightSwitch, INPUT_PULLUP);
  for (int i=1; i<3; i++){
    pinMode(switches[i], INPUT_PULLUP);
  }
  for (int i=0; i<2; i++){
    pinMode(triggers[i], INPUT_PULLUP);
  }
  
  for (int i=0; i<4; i++){
    pinMode(dirBtns[i], INPUT_PULLUP);
  }
  for (int i=0; i<4; i++){
    pinMode(btns[i], INPUT_PULLUP);
  }
  pinMode(pot, INPUT_PULLUP);
  
  for (int i=0; i<2; i++){
    pinMode(StickL[i], INPUT);
    pinMode(StickR[i], INPUT);
  }

  //Set up LED
  delay(2000);
  FastLED.addLeds<WS2812, 18>(led, 1);
  
  //Set up communication
  setLED(150, 0, 0);
  delay(1000);
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(124);

  //Open pipe
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);
  radio.startListening();

  //Waits until device is on
  setLED(80, 18, 2);
  bool Ready = false;
  while (!Ready){
    if (radio.available()) {
      while (radio.available()) {
        radio.read(&miscByte, sizeof(byte));
      }
      Ready = miscByte > 0;
    }
    delay(2);
  }
  radio.stopListening();
  setLED(0, LEDbrightness, 0);
}

void loop(){
  //Set the LED
  if (!aborted) {
    if (!digitalRead(Standby)){
      setLED(0, 0, LEDbrightness);
      aborted = false;
    } else if (!digitalRead(ABORT)) {
      setLED(LEDbrightness, 0, 0);
      aborted = true;
    } else {
      setLED(0, LEDbrightness, 0);
    }
  }
  
  /* Get input */
  //Get joystick input, it is put between 0-254
  sendData[0] = toInt(analogRead(StickL[0])/4.03); //x
  sendData[1] = toInt(analogRead(StickL[1])/4.03); //y
  sendData[2] = toInt(analogRead(StickR[0])/4.03); //r
  sendData[3] = toInt(analogRead(StickR[1])/4.03); //z
  //Get potentiometer input, it is put between 0-255
  sendData[4] = toInt(analogRead(pot)/4.01);
  
  //Get button inputs
  for (int i=0; i<4; i++){
    bitWrite(btnByte, i, !digitalRead(dirBtns[i]));
  }
  bitWrite(btnByte, 4, !digitalRead(btns[0]));
  bitWrite(btnByte, 5, !digitalRead(btns[1]));
  bitWrite(btnByte, 6, !digitalRead(triggers[0]));
  bitWrite(btnByte, 7, !digitalRead(triggers[1]));
  sendData[5] = btnByte;
  
  //Get inputs for miscByte
  bitWrite(miscByte, 0, !digitalRead(ABORT));
  bitWrite(miscByte, 1, !digitalRead(Standby));
  if (!digitalRead(lightSwitch)) {
    bitWrite(miscByte, 2, digitalRead(lightBtn));
  } else {
    bitWrite(miscByte, 2, !digitalRead(lightBtn));
  }
  for (int i=1; i<3; i++){
    bitWrite(miscByte, i+3, !digitalRead(switches[i]));
  }
  bitWrite(miscByte, 6, !digitalRead(StickL[2]));
  bitWrite(miscByte, 7, !digitalRead(StickR[2]));
  sendData[6] = miscByte;

  /* Send data */
  radio.write(&sendData, sizeof(char[7]));
}
