#include <Arduino.h>
#include <analogWrite.h>
#include "fwupdate.h"
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
//- - - - - - - - - - pin designations  - - - - - - - - - -
const int LPwm3 = 12;    // I/O channel setup ESP32 pin
const int RPwm3 = 13;    // I/O chennel setup ESP32 pin
const int LPwm1 = 26;
const int RPwm1 = 27;
const int LPwm2 = 22;
const int RPwm2 = 21;
const int LPwm4 = 4;
const int RPwm4 = 15;
const int buzz = 16;
const int spsw = 25;
const int tem = 35;
//const int spd_sw = 34;

//const int En_LR = 22;    // I/O pin for BST l_EN & R_EN (enable)
const int Vx  = 32;    // joystick connection
const int Vy  = 33;
int motorSpeed1;
int motorSpeed2;
int Xmapped, Ymapped;
//- - - - - - - - - - inital values - - - - - - - - - -
// potentiometer value
int X_axis ;         //  potentiometer converted value
int Y_axis ;
int xaxis_lora , yaxis_lora;
int Direction = 2;        // direction 1=Left turn 2=stop 3=right turn
int previous_dir = 2;
int Speed = 255;
int spmin = 15;

int Xupp = 2600;                //X_axis upper &
int Xlow = 1200;                //lower

int Ylow = 1200;                //X_axis upper &
int Yupp = 2600;                //lower

int xl_lora = 1200, xu_lora = 2600;
int yl_lora = 1200, yu_lora = 2600;
//int cont1 = 0;
//int cont2 = 0;
int cnt = 0;
int speed_h1 = 0;
int speed_h2 = 0;
int speed_h3 = 0;
int c = 0;
int speed_h4 = 0;
int motorSpeed3, motorSpeed4;
int nss = 5;
int RESET = 14;
int dio0 = 2;
const char* ssid     = "Office 7575";
const char* password = "office@7575";
String full_string;
String inString = "";
int hsb = 0;//counter for speed but.
int tempthresh = 3500;//temp_thresh

void setup()
{
  pinMode(LPwm1, OUTPUT);
  pinMode(RPwm1, OUTPUT);
  pinMode(LPwm2, OUTPUT);
  pinMode(RPwm2, OUTPUT);
  pinMode(LPwm3, OUTPUT);
  pinMode(RPwm3, OUTPUT);
  pinMode(LPwm4, OUTPUT);
  pinMode(RPwm4, OUTPUT);

  pinMode(buzz, OUTPUT);
  pinMode(spsw, INPUT_PULLUP);
  pinMode(tem, INPUT);
  //    pinMode(En_LR ,OUTPUT); // declare pin as output
  LoRa.setPins(nss, RESET, dio0);
  Serial.begin(9600);
  //  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  //  display.clearDisplay();
  //  pinMode(led,OUTPUT);
  while (!Serial);
  Serial.println("LoRa Receiver");
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");

    while (1);
  }
  LoRa.setSyncWord(0xF3);
  LoRa.setSpreadingFactor(12);
  LoRa.setSignalBandwidth(250E3);
  Serial.println("LoRa Initializing OK!");

  Serial.print("Finding wifi -  ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  int fail = 0;

  while (!(WiFi.status() == WL_CONNECTED)) {
    fail = fail + 1;
    if (fail > 22) {
      //        ESP.restart();
      //        return;
      break;
    }
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    if (FirmwareVersionCheck() == 1) {
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      delay(1000);
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      firmwareUpdate();
    } else if (FirmwareVersionCheck() == 0) {
      Serial.print("latest fw_1");
    }
    else if (FirmwareVersionCheck() == 2) {
      Serial.print("latest fw_2");
    }
  }
  handle_mspeed(0, 0, 0, 0, 0, 0, 0, 0);
  digitalWrite(buzz, HIGH);
  delay(100);
  digitalWrite(buzz, LOW);

  //  delay(5000);
}
void loop()
{
  //  Serial.println(analogRead(tem));
  if (analogRead(tem) > tempthresh) {
    Serial.println("------ over heat - stoping sys. -------");
    digitalWrite(buzz, HIGH);
    delay(500);
    digitalWrite(buzz, LOW);
    delay(500);
    handle_mspeed(0, 0, 0, 0, 0, 0, 0, 0);
    return;
  }

  for (int i = 0; i < 20; i++) {
    Y_axis = Y_axis + analogRead(Vy); // Read pot
    X_axis = X_axis + analogRead(Vx);
  }
  Y_axis = Y_axis / 20;
  X_axis = X_axis / 20;
  //  Serial.print("X_axis - ");
  //  Serial.println(X_axis);
  //  Serial.print("Y_axis - ");
  //  Serial.println(Y_axis);
  //  Serial.println("    ");


  handle_sbutt();


  //  if (spd_sw = LOW) {
  //    Speed = 200;
  //  }
  //  else {
  //    Speed = 255;
  //  }
  if ((Y_axis > Ylow && Y_axis < Yupp) && (X_axis > Xlow && X_axis < Xupp)) { //stop
    Direction = 2;
  }
  else if (Y_axis > Yupp && (X_axis > Xlow && X_axis < Xupp)) {      // extreme left
    Direction = 1;
  }
  else if (Y_axis < Ylow && (X_axis > Xlow && X_axis < Xupp)) {   //extreme right
    Direction = 3;
  }


  else if (X_axis < Xlow && (Y_axis > Ylow && Y_axis < Yupp)) {     //backward
    Direction = 4;

  }
  else if (X_axis > Xupp && (Y_axis > Ylow && Y_axis < Yupp)) { //forward
    Direction = 5;

  }

  else if (Y_axis == 4095 && (X_axis > Xupp && X_axis < 4095)) {
    //for left1
    Direction = 10;

  }
  else if ((Y_axis > Xupp && Y_axis < 4095) && X_axis == 4095) {  //changes made
    //forward left 2//
    Direction = 6;

  }

  else if ((Y_axis < Ylow && Y_axis > 0) && X_axis == 4095) {    // changes made
    //forward right 2//
    Direction = 7;
  }

  else if (Y_axis == 0 && (X_axis < 4095 && X_axis > Xupp)) { // changes made
    //forward right 1//
    Direction = 11;

  }

  else if ((Y_axis > Yupp && Y_axis < 4095) && X_axis == 0) {
    //backward left2//
    Direction = 8;

  }
  else if (Y_axis == 4095 && (X_axis > 0 && X_axis < Xlow)) {
    //back left1
    Direction = 12;

  }
  else if ((Y_axis < Ylow && Y_axis > 0) && X_axis == 0) {
    //backward right2
    Direction = 9;

  }
  else if (Y_axis == 0 && (X_axis < Xlow && X_axis > 0)) { // changes made
    //back right 1//
    Direction = 13;

  }

  if (Direction != 2) {
    c = 0;
  }
  if (Direction == 1) {               // Direction 1 take left turn
    goleft();
  }
  else if (Direction == 3) {               // Direction 3 take right turn
    goright();
  }
  else if (Direction == 2) {               // Direction 2 stop all
    stopp();
  }
  else if (Direction == 4) {                 // Direction 4 backward
    gobaack();
  }
  else if (Direction == 5) {                 // Direction 5 forward
    forward();
  }
  else if (Direction == 6) {                 // Direction 5 forward
    for_left2();
  }
  else if (Direction == 7) {                 // Direction 5 forward
    for_right2();
  }
  else if (Direction == 8) {                 // Direction 5 forward
    back_left2();
  }
  else if (Direction == 9) {                 // Direction
    back_right2();
  }
  else if (Direction == 10) {
    for_left1();
  }
  else if (Direction == 11) {
    for_right1();
  }
  else if (Direction == 12) {
    back_left1();
  }
  else if (Direction == 13) {
    back_right1();
  }
  else {
    Serial.println("missed--");
  }
  Y_axis = 0;
  X_axis = 0;
  //  delay(500);


  //  cont1 = 0;
}

int prer[4];
int prel[4];
int accd = 500;
int difs[8];

void handle_mspeed(int r1, int l1, int r2, int l2, int r3, int l3, int r4, int l4) {

  if (r1 > 0 && l1 > 0) {
    Serial.println("Protected m1");
    return;
  }
  if (r2 > 0 && l2 > 0) {
    Serial.println("Protected m2");
    return;
  }
  if (r3 > 0 && l3 > 0) {
    Serial.println("Protected m3");
    return;
  }
  if (r4 > 0 && l4 > 0) {
    Serial.println("Protected m4");
    return;
  }

  if (r1 < 0 || l1 < 0) {
    Serial.println("Protected m1");
    Serial.println("RPwm1");
    Serial.println(r1);
    Serial.println("LPwm1:");
    Serial.println(l1);
    return;
  }
  if (r2 < 0 || l2 < 0) {
    Serial.println("Protected m2");
    Serial.println("RPwm2");
    Serial.println(r2);
    Serial.println("LPwm2:");
    Serial.println(l2);
    return;
  }
  if (r3 < 0 || l3 < 0) {
    Serial.println("Protected m3");
    Serial.println("RPwm3");
    Serial.println(r3);
    Serial.println("LPwm3:");
    Serial.println(l3);
    return;
  }
  if (r4 < 0 || l4 < 0) {
    Serial.println("Protected m4");
    Serial.println("RPwm4");
    Serial.println(r4);
    Serial.println("LPwm4");
    Serial.println(l4);
    return;
  }

  difs[0] = r1 - prer[0];
  difs[1] = l1 - prel[0];
  difs[2] = r2 - prer[1];
  difs[3] = l2 - prel[1];
  difs[4] = r3 - prer[2];
  difs[5] = l3 - prel[2];
  difs[6] = r4 - prer[3];
  difs[7] = l4 - prel[3];

  //  Serial.println("RPwm1");
  //  Serial.println(r1);
  //  Serial.println("LPwm1:");
  //  Serial.println(l1);
  //  Serial.println("RPwm2");
  //  Serial.println(r2);
  //  Serial.println("LPwm2");
  //  Serial.println(l2);
  //  Serial.println("RPwm3");
  //  Serial.println(r3);
  //  Serial.println("LPwm3:");
  //  Serial.println(l3);
  //  Serial.println("RPwm4");
  //  Serial.println(r4);
  //  Serial.println("LPwm4");
  //  Serial.println(l4);
  //  Serial.println("");
  //  Serial.println("");
  //  Serial.println("");
  //  Serial.println("------------------------------");
  //  Serial.println("");
  //  Serial.println("");
  //  Serial.println("");
  for (int i = 0; i <= 255; i++) {
    if (difs[0] < 0) {
      if (prer[0] != r1) {
        prer[0]--;
        //        Serial.print("rp1 - ");
        //        Serial.println(prer[0]);
        analogWrite(RPwm1, prer[0]);
      }
    }
    if (difs[1] < 0) {
      if (prel[0] != l1) {
        prel[0]--;
        //        Serial.print("lp1 - ");
        //        Serial.println(prel[0]);
        analogWrite(LPwm1, prel[0]);
      }
    }
    if (difs[2] < 0) {
      if (prer[1] != r2) {
        prer[1]--;
        analogWrite(RPwm2, prer[1]);
      }
    }
    if (difs[3] < 0) {
      if (prel[1] != l2) {
        prel[1]--;
        analogWrite(LPwm2, prel[1]);
      }
    }
    if (difs[4] < 0) {
      if (prer[2] != r3) {
        prer[2]--;
        analogWrite(RPwm3, prer[2]);
      }
    }
    if (difs[5] < 0) {
      if (prel[2] != l3) {
        prel[2]--;
        analogWrite(LPwm3, prel[2]);
      }
    }
    if (difs[6] < 0) {
      if (prer[3] != r4) {
        prer[3]--;
        analogWrite(RPwm4, prer[3]);
      }
    }
    if (difs[7] < 0) {
      if (prel[3] != l4) {
        prel[3]--;
        analogWrite(LPwm4, prel[3]);
      }
    }
    delayMicroseconds(accd);
  }

  for (int i = 0; i <= 255; i++) {
    if (difs[0] > 0) {
      if (prer[0] != r1) {
        prer[0]++;
        //        Serial.print("rp1 - ");
        //        Serial.println(prer[0]);
        difs[1] = 0;
        analogWrite(LPwm1, 0);
        analogWrite(RPwm1, prer[0]);
      }
    }
    if (difs[1] > 0) {
      if (prel[0] != l1) {
        prel[0]++;
        //        Serial.print("lp1 - ");
        //        Serial.println(prel[0]);
        difs[0] = 0;
        analogWrite(RPwm1, 0);
        analogWrite(LPwm1, prel[0]);
      }
    }
    if (difs[2] > 0) {
      if (prer[1] != r2) {
        prer[1]++;
        difs[3] = 0;
        analogWrite(LPwm2, 0);
        analogWrite(RPwm2, prer[1]);
      }
    }
    if (difs[3] > 0) {
      if (prel[1] != l2) {
        prel[1]++;
        difs[2] = 0;
        analogWrite(RPwm2, 0);
        analogWrite(LPwm2, prel[1]);
      }
    }
    if (difs[4] > 0) {
      if (prer[2] != r3) {
        prer[2]++;
        difs[5] = 0;
        analogWrite(LPwm3, 0);
        analogWrite(RPwm3, prer[2]);
      }
    }
    if (difs[5] > 0) {
      if (prel[2] != l3) {
        prel[2]++;
        difs[4] = 0;
        analogWrite(RPwm3, 0);
        analogWrite(LPwm3, prel[2]);
      }
    }
    if (difs[6] > 0) {
      if (prer[3] != r4) {
        prer[3]++;
        difs[7] = 0;
        analogWrite(LPwm4, 0);
        analogWrite(RPwm4, prer[3]);
      }
    }
    if (difs[7] > 0) {
      if (prel[3] != l4) {
        prel[3]++;
        difs[6] = 0;
        analogWrite(RPwm4, 0);
        analogWrite(LPwm4, prel[3]);
      }
    }
    delayMicroseconds(accd);
  }

  //  if (prer[0] < r1) {
  //    for (int i = prer[0]; i <= r1; i++) {
  //      difs[0] = r1 - prer[0];
  //      analogWrite(RPwm1, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prer[0]; i >= r1 ; i--) {
  //      difs[0] = r1 - prer[0];
  //      analogWrite(RPwm1, i);
  //      delay(accd);
  //    }
  //  }
  //  if (prel[0] < l1) {
  //    for (int i = prel[0]; i <= l1; i++) {

  //      analogWrite(LPwm1, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prel[0]; i >= l1 ; i--) {
  //      difs[1] = l1 - prel[1];
  //      analogWrite(LPwm1, i);
  //      delay(accd);
  //    }
  //  }
  //
  //
  //  if (prer[1] < r2) {
  //    for (int i = prer[1]; i <= r2; i++) {

  //      analogWrite(RPwm2, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prer[1]; i >= r2 ; i--) {
  //      difs[2] = r2 - prer[1];
  //      analogWrite(RPwm2, i);
  //      delay(accd);
  //    }
  //  }
  //  if (prel[1] < l2) {
  //    for (int i = prel[1]; i <= l2; i++) {

  //      analogWrite(LPwm2, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prel[1]; i >= l2 ; i--) {
  //      difs[3] = l2 - prel[1];
  //      analogWrite(LPwm2, i);
  //      delay(accd);
  //    }
  //  }
  //
  //
  //  if (prer[2] < r3) {
  //    for (int i = prer[2]; i <= r3; i++) {

  //      analogWrite(RPwm3, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prer[2]; i >= r3 ; i--) {
  //      difs[4] = r3 - prer[2];
  //      analogWrite(RPwm3, i);
  //      delay(accd);
  //    }
  //  }
  //  if (prel[2] < l3) {
  //    for (int i = prel[2]; i <= l3; i++) {

  //      analogWrite(LPwm3, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prel[2]; i >= l3 ; i--) {
  //      difs[5] = l3 - prel[2];
  //      analogWrite(LPwm3, i);
  //      delay(accd);
  //    }
  //  }
  //
  //
  //  if (prer[3] < r4) {
  //    for (int i = prer[3]; i <= r4; i++) {

  //      analogWrite(RPwm4, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prer[3]; i >= r4 ; i--) {
  //      difs[6] = r4 - prer[3];
  //      analogWrite(RPwm4, i);
  //      delay(accd);
  //    }
  //  }
  //  if (prel[3] < l4) {
  //    for (int i = prel[3]; i <= l4; i++) {

  //      analogWrite(LPwm4, i);
  //      delay(accd);
  //    }
  //  } else {
  //    for (int i =  prel[3]; i >= l4 ; i--) {
  //      difs[7] = l4 - prel[3];
  //      analogWrite(LPwm4, i);
  //      delay(accd);
  //    }
  //  }

  //  analogWrite(RPwm1, r1);
  //  analogWrite(LPwm1, l1);
  //  analogWrite(RPwm2, r2); //b         //CW
  //  analogWrite(LPwm2, l2);
  //  analogWrite(RPwm3, r3);
  //  analogWrite(LPwm3, l3);
  //  analogWrite(RPwm4, r4);
  //  analogWrite(LPwm4, l4);

  prer[0] = r1;
  prer[1] = r2;
  prer[2] = r3;
  prer[3] = r4;

  prel[0] = l1;
  prel[1] = l2;
  prel[2] = l3;
  prel[3] = l4;
}
void handle_loram() {
  if (yaxis_lora == 5000 && xaxis_lora == 5000) { //stop
    c = 0;
    handle_mspeed(0, 0, 0, 0, 0, 0, 0, 0);
    Direction = 2;
    Serial.println("===============   stoped  ============================");
  }
  else if (yaxis_lora == -2 && xaxis_lora == -2) { //missed
    //          c = 0;
    Direction = previous_dir;
  }
  else if ((yaxis_lora > 0 && xaxis_lora > 0) || (yaxis_lora == 0 || xaxis_lora == 0)) {

    if (yaxis_lora > yu_lora && (xaxis_lora > xl_lora && xaxis_lora < xu_lora)) {      // extreme left
      Direction = 1;
      previous_dir = Direction ;
    }
    else if (yaxis_lora < yl_lora && (xaxis_lora > xl_lora && xaxis_lora < xu_lora)) {   //extreme right
      Direction = 3;
      previous_dir = Direction ;
    }

    else if (xaxis_lora < xl_lora && (yaxis_lora > yl_lora && yaxis_lora < yu_lora)) {     //backward
      Direction = 4;
      previous_dir = Direction ;
    }
    else if (xaxis_lora > xu_lora && (yaxis_lora > yl_lora && yaxis_lora < yu_lora)) { //forward
      Direction = 5;
      previous_dir = Direction ;
      //      Serial.println("forward lora");
    }

    else if (yaxis_lora == 4095 && (xaxis_lora > xu_lora && xaxis_lora < 4095)) {
      //for left1
      Direction = 10;
      previous_dir = Direction ;
    }
    else if ((yaxis_lora > yu_lora && yaxis_lora < 4095) && xaxis_lora == 4095) {  //changes made
      //forward left 2//
      Direction = 6;
      previous_dir = Direction ;
    }

    else if ((yaxis_lora < yu_lora && yaxis_lora > 0) && xaxis_lora == 4095) {    // changes made
      //forward right 2//
      Direction = 7;
      previous_dir = Direction ;
    }

    else if (yaxis_lora == 0 && (xaxis_lora < 4095 && xaxis_lora > xu_lora)) { // changes made
      //forward right 1//
      Direction = 11;
      previous_dir = Direction ;
    }

    else if ((yaxis_lora > yu_lora && yaxis_lora < 4095) && xaxis_lora == 0) {
      //backward left2//
      Direction = 8;
      previous_dir = Direction ;
    }
    else if (yaxis_lora == 4095 && (xaxis_lora > 0 && xaxis_lora < xl_lora)) {
      //back left1
      Direction = 12;
      previous_dir = Direction ;
    }
    else if ((yaxis_lora < yl_lora && yaxis_lora > 0) && xaxis_lora == 0) {
      //backward right2
      Direction = 9;
      previous_dir = Direction ;
    }
    else if (yaxis_lora == 0 && (xaxis_lora < xl_lora && xaxis_lora > 0)) { // changes made
      //back right 1//
      Direction = 13;
      previous_dir = Direction ;
    } else {
      Serial.println("No dir found ");
    }
  }
  else {
    Serial.println("Nothing");
  }


  if (Direction == 1) {               // Direction 1 take left turn
    goleft();
  }
  else if (Direction == 3) {               // Direction 3 take right turn
    goright();
  }

  else if (Direction == 4) {                 // Direction 4 backward
    gobaack();
  }
  else if (Direction == 5) {                 // Direction 5 forward
    forward();
  }
  else if (Direction == 6) {                 // Direction 5 forward
    for_left2();
  }
  else if (Direction == 7) {                 // Direction 5 forward
    for_right2();
  }
  else if (Direction == 8) {                 // Direction 5 forward
    back_left2();
  }
  else if (Direction == 9) {                 // Direction
    back_right2();
  }
  else if (Direction == 10) {
    for_left1();
  }
  else if (Direction == 11) {
    for_right1();
  }
  else if (Direction == 12) {
    back_left1();
  }
  else if (Direction == 13) {
    back_right1();
  }
  else {
    Serial.println("missed--");
  }
}
void goleft() {
  if (c == 1) {
    Ymapped = map(yaxis_lora, yu_lora, 4095, 0, 10000);
    motorSpeed3 = map(Ymapped, 0, 10000, 0, Speed);
    motorSpeed4 = 0;
    handle_mspeed(0, motorSpeed3, motorSpeed3, 0, 0, motorSpeed3, motorSpeed3, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("Left--lora");
  }
  else {
    Ymapped = map(Y_axis, Yupp, 4095, 0, 10000);
    motorSpeed1 = map(Ymapped, 0, 10000, 0, Speed);
    motorSpeed2 = 0;
    handle_mspeed(0, motorSpeed1, motorSpeed1, 0, 0, motorSpeed1, motorSpeed1, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("Left");
  }
}

void goright() {
  if (c == 1) {
    Ymapped = map(yaxis_lora, yl_lora, 0, 0, 10000);
    motorSpeed4 = map(Ymapped, 0, 10000, 0, Speed);
    motorSpeed3 = 0;
    handle_mspeed(motorSpeed4, 0, 0, motorSpeed4, motorSpeed4, 0, 0, motorSpeed4); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("right--lora");
  }
  else {
    Xmapped = map(Y_axis, Ylow, 0, 0, 10000);
    motorSpeed2 = map(Xmapped, 0, 10000, 0, Speed);
    motorSpeed1 = 0;
    handle_mspeed(motorSpeed2, 0, 0, motorSpeed2, motorSpeed2, 0, 0, motorSpeed2); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("right");
  }

}
void stopp() {
  //  if(cnt==2){
  motorSpeed1 = 0;
  motorSpeed2 = 0;
  // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
  //  Serial.println("stop");
  int packetSize = LoRa.parsePacket();
  full_string = "";
  if (packetSize)
  {
    // received a paket
    Serial.println("");
    Serial.println("...................................");
    Serial.println("Received packet: ");
    while (LoRa.available()) {
      char incoming = (char)LoRa.read();
      full_string += incoming;
    }
    String data1 = full_string.substring(0, 4);
    String data2 = full_string.substring(5, 12);
    xaxis_lora = data1.toInt();
    yaxis_lora = data2.toInt();
    Serial.print("Xlora: ");
    Serial.println(xaxis_lora);
    Serial.print("Ylora: ");
    Serial.println(yaxis_lora);
    c = 1;
    if (yaxis_lora == 5000 && xaxis_lora == 5000) {
      Serial.println("==============================  stop recived ================");
      c = 0;
    }

    handle_loram();

  }
  else {
    if (yaxis_lora != 5000 && xaxis_lora != 5000) {
      xaxis_lora = -2;
      yaxis_lora = -2;
      Direction = previous_dir;
      //      handle_loram();
      //          c=0;
    }
    if (c == 0) {
      handle_mspeed(0, 0, 0, 0, 0, 0, 0, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4

    }

    //    return;
  }
  Serial.print("C: ");
  Serial.println(c);
  Serial.print("Direction: ");
  Serial.println(Direction);
}





void forward() {
  if (c == 1) {
    Xmapped = map(xaxis_lora, xu_lora, 4095, 0, 10000);
    Serial.print("xaxis_lora - ");
    Serial.println(xaxis_lora);
    Serial.print("Xmapped - ");
    Serial.println(Xmapped);
    motorSpeed3 = map(Xmapped, 0, 10000, 0, Speed);
    Serial.print("motorSpeed3 - ");
    Serial.println(motorSpeed3);
    motorSpeed4 = map(Xmapped, 0, 10000, 0, Speed);
    Serial.print("motorSpeed4 - ");
    Serial.println(motorSpeed4);
    handle_mspeed( motorSpeed3, 0, motorSpeed4, 0, motorSpeed3, 0, motorSpeed4, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("forward--lora");
  }
  else {
    Xmapped = map(X_axis, Xupp, 4095, 0, 10000);
    motorSpeed1 = map(Xmapped, 0, 10000, 0, Speed);
    motorSpeed2 = map(Xmapped, 0, 10000, 0, Speed);
    handle_mspeed( motorSpeed1, 0, motorSpeed2, 0, motorSpeed1, 0, motorSpeed2, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("forward");
  }
}
void gobaack() {
  if (c == 1) {
    Xmapped = map(xaxis_lora, xl_lora, 0, 0, 10000);
    motorSpeed3 = map(Xmapped, 0, 10000, 0, Speed);
    motorSpeed4 = map(Xmapped, 0, 10000, 0, Speed);
    handle_mspeed( 0, motorSpeed3, 0, motorSpeed4, 0, motorSpeed3, 0, motorSpeed4); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("back--lora");
  }
  else {
    Xmapped = map(X_axis, Xlow, 0, 0, 10000);
    motorSpeed1 = map(Xmapped, 0, 10000, 0, Speed);
    motorSpeed2 = map(Xmapped, 0, 10000, 0, Speed);
    handle_mspeed(0, motorSpeed1, 0, motorSpeed2,  0, motorSpeed1, 0, motorSpeed2); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("back");
  }
}
void for_left2() {
  if (c == 1) {
    Ymapped = map(yaxis_lora, yu_lora, 4095, 0, 10000);
    motorSpeed4 = map(Ymapped, 0, 10000, 100, Speed);
    handle_mspeed(100, 0, motorSpeed4, 0, 100, 0, motorSpeed4, 0);
    Serial.println("for_left2--lora");
  }
  else {
    Ymapped = map(Y_axis, Yupp, 4095, 0, 10000);
    //  motorSpeed1 = map(Y_axis, 0, 4095, Speed, 0);
    motorSpeed2 = map(Ymapped, 0, 10000, 100, Speed);
    handle_mspeed(100, 0, motorSpeed2, 0, 100, 0, motorSpeed2, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    //handle_mspeed( motorSpeed2, 0,100, 0, 100, 0, motorSpeed2, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("for_left2");
  }
}
void for_left1() {
  if (c == 1) {
    Xmapped = map(xaxis_lora, xu_lora, 4095, 0, 10000);
    motorSpeed4 = map(Xmapped, 0, 10000, Speed, spmin);
    handle_mspeed(0, 100, motorSpeed4, 0, 0, 100, motorSpeed4, 0);
    Serial.println("for_left1--lora");
  }
  else {
    //  Xmapped = map(Y_axis, Ylow, 0, 0, 4095);
    Xmapped = map(X_axis, Xupp, 4095, 0, 10000);
    motorSpeed2 = map(Xmapped, 0, 10000, Speed, spmin);
    handle_mspeed(0, 100, motorSpeed2, 0, 0, 100, motorSpeed2, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("for_left1");
  }
}
void for_right2() {
  if (c == 1) {
    Ymapped = map(yaxis_lora, yl_lora, 0, 0, 10000);
    motorSpeed3 = map(Ymapped, 0, 10000, 100, Speed);
    handle_mspeed(motorSpeed3, 0, 100, 0, motorSpeed3, 0, 100, 0);
    Serial.println("for_right2--lora");
  }
  else {
    //  Xmapped = map(Y_axis, Yupp, 4095, 0, 10000);
    Ymapped = map(Y_axis, Ylow, 0, 0, 10000);
    motorSpeed1 = map(Ymapped, 0, 10000, 100, Speed);
    handle_mspeed(motorSpeed1, 0, 100, 0, motorSpeed1, 0, 100, 0); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("for_right--2");
  }
}
void for_right1() {
  if (c == 1) {
    Xmapped = map(xaxis_lora, xu_lora, 4095, 0, 10000);
    motorSpeed3 = map(Xmapped, 0, 10000, Speed, spmin);
    handle_mspeed(motorSpeed3, 0, 0, 100, motorSpeed3, 0, 0, 100);
    Serial.println("for_right1--lora");
  }
  else {
    //    Xmapped = map(Y_axis, Yupp, 4095, 0, 10000);
    Xmapped = map(X_axis, Xupp, 4095, 0, 10000);
    motorSpeed1 = map(Xmapped, 0, 10000, Speed, spmin);
    handle_mspeed(motorSpeed1, 0, 0, 100, motorSpeed1, 0, 0, 100); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("for_right--1");
  }
}
void back_left1() {
  if (c == 1) {
    Xmapped = map(xaxis_lora, xl_lora, 0, 0, 10000);
    motorSpeed4 = map(Xmapped, 0, 10000, spmin, 80);
    handle_mspeed(0, motorSpeed4, 0, 100, 0, motorSpeed4, 0, 100);
    Serial.println("back_left1--lora");
  }
  else {
    Xmapped = map(X_axis, Xlow, 0, 0, 10000);
    motorSpeed2 = map(Xmapped, 0, 10000, spmin, 80);
    handle_mspeed(0, motorSpeed2, 0, 100, 0, motorSpeed2, 0, 100); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("back_left1");
  }
}

void back_left2() {
  if (c == 1) {
    Ymapped = map(yaxis_lora, yu_lora, 4095, 0, 10000);
    motorSpeed4 = map(Ymapped, 0, 10000, Speed, 100);
    handle_mspeed(0, motorSpeed4, 0, 200, 0, motorSpeed4, 0, 200);
    Serial.println("back_left2--lora");
  }
  else {
    Ymapped = map(Y_axis, Yupp, 4095, 0, 10000);
    motorSpeed2 = map(Ymapped, 0, 10000, Speed, 100);
    handle_mspeed(0, motorSpeed2, 0, 200, 0, motorSpeed2, 0, 200); // RPwm1,LPwm1,RPwm2,LPwm2,RPwm3,LPwm3,RPwm4,LPwm4
    Serial.println("back_left2");
  }
}
void back_right1() {
  if (c == 1) {
    Xmapped = map(xaxis_lora, xl_lora, 0, 0, 10000);
    motorSpeed3 = map(Xmapped, 0, 10000, spmin, 80);
    handle_mspeed(0, 100, 0, motorSpeed3, 0, 100, 0, motorSpeed3);
    Serial.println("back_right1--lora");
  }
  else {
    Xmapped = map(X_axis, Xlow, 0, 0, 10000);
    motorSpeed1 = map(Xmapped, 0, 10000, spmin, 80);
    handle_mspeed(0, 100, 0, motorSpeed1, 0, 100, 0, motorSpeed1);
    Serial.println("back_right1");
  }
}
void back_right2() {
  if (c == 1) {
    Ymapped = map(yaxis_lora, 1700, 0, 0, 10000);
    motorSpeed3 = map(Ymapped, 0, 10000, Speed, 100);
    handle_mspeed(0, 200, 0, motorSpeed3, 0, 200, 0, motorSpeed3);
    Serial.println("back_right2--lora");
  }
  else {
    Ymapped = map(Y_axis, 1700, 0, 0, 10000);
    motorSpeed1 = map(Ymapped, 0, 10000, Speed, 100);
    handle_mspeed(0, 200, 0, motorSpeed1, 0, 200, 0, motorSpeed1);
    Serial.println("back_right2");
  }
}//joytest3

void handle_sbutt() {
  if (digitalRead(spsw) == LOW) {// Pos - top - high speed
    if (hsb == 0) {
      hsb = 1;
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      delay(50);
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
    }
    Speed = 255;
  }
  else {
    if (hsb == 1) {
      hsb = 0;
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
      delay(2000);
      digitalWrite(buzz, HIGH);
      delay(100);
      digitalWrite(buzz, LOW);
    }
    Speed = 190;
  }
}
