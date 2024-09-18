#include <TimerOne.h>

#define pwmch 9               //ขา PWM
#define senpv A1              //อ่าน PV
#define senvoltout A0         //อ่าน Volte OUT
#define sensetvoltout A2      //เซ็ต Volte OUT

const float clock = F_CPU / 1000000.0;
const float khz = 62.5; // กำหนด pwm กี่ khz
const float frequency = (clock / (khz * clock)) * 1000;

float VSI = 0;  //โวลต์เซ็นอินพุต
float VSO = 0;  //โวลต์เซ็นเอ้าพุต
float PV = 0;   //โวลต์แผง
float VO = 0;   //โวลต์ออก
int SetVO = 0;  //เซ็ตโวลต์ออก
int avgvolte = 3;
float rvef = (1023.0 / 5.0); //1024-1
int PWM = 0;
int maxPWM;
int PWMLimit = 1023 * 0.75; //PFC *0.3 // solar 0.75
int MAXVO = 400;

void setup() {
  pinMode(pwmch, OUTPUT);
  pinMode(senvoltout, INPUT);
  pinMode(senvoltout, INPUT);
  pinMode(sensetvoltout, INPUT);
  setpwm();
  Readsen();
  //soft start
  maxPWM = (1-(PV / SetVO)) * 1023;
  while (VO < SetVO || PWM < maxPWM) {
    PWM++;
    PWM = constrain(PWM, 0, PWMLimit);
    Timer1.setPwmDuty(pwmch, PWM);
    Readsen();
    if(VO > SetVO || PWM > maxPWM || VO > MAXVO){break;}
    delay(100);
  }
  PWM = 0;

}

void loop() {
  Readsen();
  Bulk();
}

void Readsen(){
  VSI = 0;
  VSO = 0;
  for(int i = 0; i < avgvolte;i++){
    VSI = VSI + analogRead(senpv) / rvef; //0-1023 to 0-5v
    VSO = VSO + analogRead(senvoltout) / rvef;
  }
  PV = (VSI / avgvolte) * 101;  // R1 100k + R2 1K = 101
  VO = (VSO / avgvolte) * 101;  // R1 100k + R2 1K = 101

  //set volt out
  SetVO = (analogRead(sensetvoltout) / rvef) * 100;
  if(SetVO > 400) SetVO = 400;
}

void setpwm(){
  Timer1.initialize(frequency);
  Timer1.pwm(pwmch,0);
  Timer1.start();
}

void Bulk(){
  if(VO < MAXVO){                  // ตรวจสอบ โวลต์ออก น้อยกว่า 400V 
    if(VO > SetVO){PWM--;}         //ตรวจสอบ โวลต์ออก มากกว่า เซ็ตโวลต์ออก 380V / PWM ลด
    else if(VO < SetVO){PWM++;}    //ตรวจสอบ โวลต์ออก น้อยกว่า เซ็ตโวลต์ออก 380V / PWM เพิ่ม
  }
  else {PWM = 0;}                  // ตรวจสอบ โวลต์ออก มากกว่า 400V / PWM = 0
  PWM = constrain(PWM, 0, PWMLimit);
  Timer1.setPwmDuty(pwmch, PWM);
}
