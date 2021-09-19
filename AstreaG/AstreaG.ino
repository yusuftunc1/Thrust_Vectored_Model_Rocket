#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

int pinCE = 9;
int pinCSN = 10;

RF24 radio(pinCE,pinCSN);

const byte adress[6] = "00001";

struct dataStruct{
  double anglex;
  double angley;
  double tempature;
  float pressure;
  double Raltitude;
  double accZ;
  int control1;
  int control2;
}allData;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  radio.begin();

  radio.openReadingPipe(0,adress);

  radio.startListening();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(radio.available()){
    radio.read(&allData,sizeof(allData));
    Serial.print(allData.anglex,6);
    Serial.print("  ");
    Serial.print(allData.angley,6);
    Serial.print("  "); 
    Serial.print(allData.tempature,6);
    Serial.print("  ");   
    Serial.print(allData.pressure,6);
    Serial.print("  "); 
    Serial.print(allData.Raltitude,6);
    Serial.print("  "); 
    Serial.print(allData.accZ,6);
    Serial.print("  "); 
    Serial.print(allData.control1,6);
    Serial.print("  "); 
    Serial.print(allData.control2,6);
    Serial.println("  "); 
    delay(200);
    }
}
