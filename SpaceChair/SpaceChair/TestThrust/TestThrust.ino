/*
        Arduino Brushless Motor Control
     by Dejan, https://howtomechatronics.com
*/

#include <Servo.h>

Servo CW, CCW;     // create servo object to control the ESC

int percentCW;
int percentCCW;
int power;  // value from the analog pin

void setup() {
  Serial.begin(115200);
  // Attach the ESC on pin 9
  CW.attach(2,1000,2000); //CW (pin, min pulse width, max pulse width in microseconds) 
  CCW.attach(3,1000,2000); //CCW
}

void loop() {
  delay(200);
  percentCW = analogRead(A0);
  percentCCW = analogRead(A2);
  power = analogRead(A3);
  


  //ESC:90° is off, 0° is full CCW, 180° is full CW)
  
  //power = map(power, 0, 1023, 0, 180);   // scale it to use it with the servo library (value between 0 and 180)
  power = map(power, 0, 1023, 100, -100);
  power=37;
  percentCW=75;
  percentCCW=100;
  //percentCW = map(percentCW, 0, 1023, 100, 0);
  //percentCCW = map(percentCCW, 0, 1023, 0, 100);
  

  Serial.print("%Power: ");
  Serial.print(power);
  Serial.print("%CW: ");
  Serial.print(percentCW);
  Serial.print(" %CCW");
  Serial.println(percentCCW);

  //Serial.print("CW: ");
  //Serial.print(power*(percentCW/100.0));
  //Serial.print(" CCW: ");
  //Serial.println(-power*(percentCCW/100.0));

  //Serial.print("CW: ");
  //Serial.print(map(power*(percentCW/100.0),-100, 100, 1000, 2000));
  //Serial.print(" CCW: ");
  //Serial.println(map(power*(percentCCW/100.0),-100, 100, 1000, 2000));

  //de -100 = CCW, 0 = 0, 100 = CW
  //1000 = CCW, 1500 = 0, 2000=CW
  
  CW.writeMicroseconds(map(power*(percentCW/100.0),-100, 100, 1000, 2000));    // Send the signal to the ESC
  CCW.writeMicroseconds(map(-power*(percentCCW/100.0),-100, 100, 1000, 2000));
}
