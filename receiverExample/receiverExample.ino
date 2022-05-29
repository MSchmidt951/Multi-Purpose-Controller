#include <RF24.h>

//Program settings
const int showBinary = false;
const int showAnalogue = false;
const int showDigital = true;

//Radio vars
RF24 radio(6, 7); //Sets CE and CSN pins of the radio, can be set to any pin
byte addresses[][6] = {"C", "D"};
char rawData[7]; //Raw input data from radio
byte Data[7]; //Usable input data from radio

void checkData(int byteIndex, int index, String inputName) {
  if (bitRead(Data[byteIndex], index)) {
    Serial.print(inputName + ", ");
  }
}

void setup(){
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  
  //Start the serial communication
  Serial.begin(57600);
  while (!Serial);
  
  //Set up communication
  Serial.print("\n\nSetting up radio ... ");
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.setChannel(124);
  
  //Open pipe
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);
  
  //Ready device
  for (int i=0; i<10; i++){
    radio.write(0b01010101, 1);
    delay(20);
  }
  radio.startListening();

  Serial.println("done!");
}

void loop(){
  if (radio.available()) {
    //Get data from controller
    while (radio.available()) {
      radio.read(&rawData, sizeof(char[7]));
    }
    for (int i=0; i<7; i++) {
      Data[i] = byte(rawData[i]);
    }

    //Print the data in binary
    if (showBinary) {
      Serial.print("Raw binary data: ");
      for (int i=0; i<7; i++) {
        Serial.print(byte(Data[i]), BIN);
        Serial.print(" ");
      }
      Serial.println();
    }

    /* Process the data */

    if (showAnalogue){
      Serial.print("Joysticks: ");
  
      //Read the horizontal axis of the left joystick
      Serial.print((Data[0] - 127)/1.27); //Range of -100 to 100
      Serial.print(" ");
  
      //Read the vertical axis of the left joystick
      Serial.print((Data[1] - 127)/1.27);
      Serial.print("  ");
  
      //Read the horizontal axis of the right joystick
      Serial.print((Data[2] - 127)/1.27);
      Serial.print(" ");
  
      //Read the vertical axis of the right joystick
      Serial.print((Data[3] - 127)/1.27);
      Serial.print("   ");
  
      //Read the potentiometer
      Serial.print("Potentiometer: ");
      Serial.println((Data[4]-3)/2.52); //Range of 0 to 100
    }

    //Read the byte containing most of the buttons and the misc data
    if (showDigital) {
      Serial.print("Controls pressed: ");
      checkData(5, 0, "left");
      checkData(5, 1, "right");
      checkData(5, 2, "up");
      checkData(5, 3, "down");
      checkData(5, 4, "general left");
      checkData(5, 5, "general right");
      checkData(5, 6, "left trigger");
      checkData(5, 7, "right trigger");
      
      checkData(6, 0, "abort");
      checkData(6, 1, "standby");
      checkData(6, 2, "light");
      checkData(6, 3, "left switch");
      checkData(6, 5, "right switch");
      checkData(6, 6, "left joystick button");
      checkData(6, 7, "right joystick button");
      
      Serial.println();
    }
  }

  delay(10);
}
