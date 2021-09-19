#include <Servo.h>
#include <MPU6050_tockn.h>
#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <Wire.h> 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);  // CE, CSN
//address through which two modules communicate.
const byte address[6] = "00001";


int servoPinX=6;  // 6 pcb
int servoPinY=5; //5 pcb
int buzzerPin=4; // 4 pcb
int redPin=8;  //7 pcb
int greenPin=7; // 8 pcb
#define pwm 3   // 3 pcb

Servo srvX;
Servo srvY;
MPU6050 mpu6050(Wire);
Adafruit_BMP085 bmp180 ;

double errorx , errory;
double lasterrorx = 0;
double lasterrory = 0;
double cp = 0.5;
double ci = 0.2;
double cd = 0.1;

double pid_ix;
double pid_iy;

double anglexkf;
double angleykf;

double tempature;
float pressure;
double Raltitude;
int control1;
int control2;

int setZero = 685;
int lastAlt ;



struct dataStruct{
  double anglex;
  double angley;
  double tempature;
  float pressure;
  double reelAlt;
  double accZ;
  int control1;
  int control2;
}allData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  srvX.attach(servoPinX);
  srvY.attach(servoPinY);
    
  pinMode(redPin,OUTPUT);
  pinMode(greenPin,OUTPUT);
  pinMode(buzzerPin,OUTPUT);
  pinMode(pwm,OUTPUT);
  
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  if (!bmp180.begin()) {
    Serial.println("begin() failed. check your BMP180 Interface and I2C Address.");
    digitalWrite(buzzerPin,HIGH);
    digitalWrite(redPin,HIGH);
    digitalWrite(greenPin,LOW);
    while (1);
  }

  radio.begin();
  radio.openWritingPipe(address);
  radio.stopListening();
}



void loop() {
  // put your main code here, to run repeatedly:
  getpressure();
  FlightCheck();
  mpu6050.update();
  double anglex = mpu6050.getAngleX();
  double angley = mpu6050.getAngleY();
  
  allData.anglex = mpu6050.getAngleX();
  allData.angley = mpu6050.getAngleY();
  errorx = anglex ;
  errory = angley ;
  if(anglex>60 || angley>60){
    digitalWrite(buzzerPin,HIGH);
    digitalWrite(redPin,HIGH);
    digitalWrite(greenPin,LOW);
    }
  else if(anglex<-60 || angley< -60){
    digitalWrite(buzzerPin,HIGH);
    digitalWrite(redPin,HIGH);
    digitalWrite(greenPin,LOW);
    }
  else {
    digitalWrite(buzzerPin,LOW);
    digitalWrite(redPin,LOW);
    digitalWrite(greenPin,HIGH);
    }
    
  if(errorx<5 && errorx> -5){
      resPidVariablesX();
      srvX.write(90);
    }
  else {
    double delta_time = 15/1000;
    double rotatex = pidCalculateX(delta_time,errorx,cp,ci,cd);  //def for servox
    rotatex = 90 - rotatex;
    if (rotatex < 60){
      srvX.write(60);
      }
    else if (rotatex > 120){
      srvX.write(120);
      }
    else{
      srvX.write(rotatex);
      }
    }
  if(errory<5 && errory> -5){
      resPidVariablesY();
      srvY.write(90);
    }
  else {
    double delta_time = 15/1000;
    double rotatey = pidCalculateY(delta_time,errory,cp,ci,cd);  //def for servoy
    rotatey = 90 + rotatey;
    if (rotatey < 60){
      srvY.write(60);
      }
    else if (rotatey > 120){
      srvY.write(120);
      }
    else{
      srvY.write(rotatey);
      }
    }
  sendData();
  delay(15);
}

int resPidVariablesX() {
  pid_ix = 0;
  lasterrorx = 0;

  }

int resPidVariablesY() {
    pid_iy = 0;
    lasterrory = 0;
  }

double pidCalculateX(double delta_time, double errorx,double cp, double ci, double cd) {
  double pid_px = errorx;
  double pid_dx = (errorx - lasterrorx)/ 0.015 ;
  pid_ix = pid_ix + (errorx* 0.015) ; 
  double rotatex = (pid_px*cp) + (pid_ix*ci) + (pid_dx*cd) ;
  lasterrorx = errorx;
  
  return rotatex ;
  }


double pidCalculateY(double delta_time,double errory,double cp,double ci,double cd){
  double pid_py = errory;
  double pid_dy = (errory - lasterrory) / 0.015;
  pid_iy = pid_iy + (errory*0.015) ;
  double rotatey = (pid_py*cp) + (pid_iy*ci) + (pid_dy*cd);
  lasterrory = errory;

  return rotatey ;
  }


void getpressure(){

  tempature = bmp180.readTemperature() ;
  pressure = bmp180.readPressure() ;
  Raltitude = bmp180.readAltitude(101500);
  allData.tempature = tempature;
  allData.pressure = pressure;
  
  }

void sendData(){
  radio.write(&allData,sizeof(allData));
  delay(100);
  }



void FlightCheck(){
  int Ralt = bmp180.readAltitude(101500);
  int reelAlt = Ralt - setZero;
  allData.reelAlt = reelAlt;
  int diff = lastAlt - reelAlt;
  double accZ = mpu6050.getAccZ() ;
  allData.accZ = accZ;
  if (diff > 2 && accZ < 0){
     Serial.println("descend started");
     control2 = 1;
     allData.control2 = control2;
     delay(2000);
     //parachute deployment
     analogWrite(pwm,225);
     digitalWrite(buzzerPin,HIGH);

     delay(2000);
     analogWrite(pwm,0);
     digitalWrite(buzzerPin,LOW);
    }
  else if ( reelAlt > 2 && accZ > 1 ){
    Serial.println("ascend started");
    control1 = 1;
    allData.control1 = control1;
    }
  else{
    control1 = 0;
    control2 = 0;
    allData.control1 = control1;
    allData.control2 = control2;
    }

  lastAlt = reelAlt;
  }

  
