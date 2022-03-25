/*
 * Fauteuil Spatial 
 * 
 * Odysseus 3.1
 */

#include <Servo.h>


//Variable declaration
//Two 3 axis joysticks are available.
//'|' axis is translation (left hand)
//'+' axis is rotation (right hand)
#define translateXpin A1
#define translateYpin A3
#define translateZpin A5
#define rotateXpin A0
#define rotateYpin A2
#define rotateZpin A4

//ESC location
#define xCWPin  2
#define xCCWPin 3
#define yCWPin  4
#define yCCWPin 5
#define zCWPin  6
#define zCCWPin 7

//Enable to inverse (swap) current axis value
//#define inverseTranslateX
//#define inverseTranslateY
#define inverseTranslateZ
//#define inverseRotateX
#define inverseRotateY
#define inverseRotateZ

//Neutral ignore
//Define how much percentage must be ignored
//around neutral
//Axis value is usually -100%<->0%<->+100%
//Then everyting arrounf -neutral%<->0%<->+neutral%
//will be ignored. Default is 8%
//Value must be between 0 and 20 maximum.
#define neutralPercent 8

//Thuster max power
//define max percentage of power
//this thruster can handle (usually
// up to 100%)
//Allow limiting Forward power to
//compensate for Reverse power
//ie: FW=70%, RV=100%
#define thrusterXForwardMax 75
#define thrusterXReverseMax 100
#define thrusterYForwardMax 75
#define thrusterYReverseMax 100
#define thrusterZForwardMax 75
#define thrusterZReverseMax 100

//Kinetic limit
//Usuually defined at 1000 (100% * 10)
#define kineticMaxLevel 1000

//Neutral limit to avoid boucing
//usually 10%
#define kineticNeutralLevel 10


/*
 * system values
 * Do NOT modify
 */ 
//Axis raw values
int translateX = 0;
int translateY = 0;
int translateZ = 0;
int rotateX = 0;
int rotateY = 0;
int rotateZ = 0;


//JoystickPercent
//Per joystick current position
//in percent (-100%<->0%<->+100%
struct joystickPercent {
  int X = 0;
  int Y = 0;
  int Z = 0;
};

//Thruster percentage
//Per axis thrust level in percent
struct thrustLevel {
    //thrust for clockwise (CW) motor
    int CW = 0;
    //thrust for counter clockwise (CCW) motor
    int CCW = 0;
    //Kinetic value for space mode
    int kineticCW = 0;
    int kineticCCW = 0;
};

//Space mode selector
bool spaceMode = false;

Servo xCW, xCCW, yCW, yCCW, zCW, zCCW;

/*
 * Temporary definitions
 * Should be removed before release
 */
//Per axis definition (could be removed when switching to functions
struct thrustLevel thrustX;
struct thrustLevel thrustY;
struct thrustLevel thrustZ;

//Per joystick definition (could be removed when switching to functions
struct joystickPercent translation;
struct joystickPercent rotation;

void setup() {
  //Initialize serial debug link
  Serial.begin(115200);

  //Initialize ESC
  xCW.attach(xCWPin,1000,2000); //X CW
  xCCW.attach(xCCWPin,1000,2000); //X CCW
  yCW.attach(yCWPin,1000,2000); //Y CW
  yCCW.attach(yCCWPin,1000,2000); //Y CCW
  zCW.attach(zCWPin,1000,2000); //Z CW
  zCCW.attach(zCCWPin,1000,2000); //Z CCW
  
}

//Convert current axis value to a real percent
int joystickToPercent (int analogValue, bool inversedAxis = false){

  int value = (100 - (constrain((analogValue),0,1000) / 5));

  //Neutral adjustement, to avoid motor spinning while joystick is around neutral position
  //will ignore some percentage (defined in neutralPercent)
  if ((value > -neutralPercent) && (value < neutralPercent) ) {
    value = 0;
  } else {
    //Recompute value to ignore neutral while driving motor smootlhy
    if ( value > 0){
      //For positive value
      value = (value * (1 + (neutralPercent * 0.01))) - neutralPercent;
    } else {
      value = (value * (1 + (neutralPercent * 0.01))) + neutralPercent;
    }
  }

  //Inverted Axis, inverse return value
  if (inversedAxis == true){
    value = value * -1;
  }

  return value;
}

//Convert joystick percentage value to real thrust order
struct thrustLevel percentToThrust (struct thrustLevel thrust, int translationAxisPercent, int rotationAxisPercent){
   //Translation thrust values
   int tCW, tCCW = 0;
   //Rotation thrust values
   int rCW, rCCW = 0;

   //Compute translation
   tCW = translationAxisPercent;
   tCCW = -translationAxisPercent;
   
   //Compute rotation
   rCW = rCCW = rotationAxisPercent;
  
   //Mix values
   //Do we are higher then max available thrust?
   if(abs(tCW)+abs(rCW) > 100){
     //lower proportionnaly max output value
     thrust.CW = (tCW+rCW)*(100.0 / (abs(tCW)+abs(rCW)));
     thrust.CCW = (tCCW+rCCW) * (100.0 / (abs(tCCW)+abs(rCCW)));
   } else {
    //We do not exceed max thrust
     thrust.CW = tCW+rCW;
     thrust.CCW = tCCW+rCCW;
   }

   return thrust;
}

//Create a fake spatial effect, when each order needs
//to be reciproced. In space we don't have water to slow us
struct thrustLevel spaceKineticThrust (struct thrustLevel spaceThrust){
 
   //Add kinetic energy
   spaceThrust.kineticCW = spaceThrust.kineticCW + spaceThrust.CW;
   spaceThrust.kineticCCW = spaceThrust.kineticCCW + spaceThrust.CCW;
   
   //We cap kinetic energy to a max (usually defined at 1000) on CW thuster
   if( (spaceThrust.kineticCW > 0 ) && (spaceThrust.kineticCW > kineticMaxLevel) ) {
    spaceThrust.kineticCW = kineticMaxLevel;
   } else if( (spaceThrust.kineticCW < 0) && (spaceThrust.kineticCW < -kineticMaxLevel) ) {
    spaceThrust.kineticCW = -kineticMaxLevel;
   }
   //Cap for neutral (easier stops)
   if( (spaceThrust.kineticCW > -kineticNeutralLevel) && (spaceThrust.kineticCW < kineticNeutralLevel) ) {
    spaceThrust.kineticCW = 0;
   }
   //For CCW
   if( (spaceThrust.kineticCCW > 0 ) && (spaceThrust.kineticCCW > kineticMaxLevel) ) {
    spaceThrust.kineticCCW = kineticMaxLevel;
   } else if( (spaceThrust.kineticCCW < 0) && (spaceThrust.kineticCCW < -kineticMaxLevel) ) {
    spaceThrust.kineticCCW = -kineticMaxLevel;
   }
   //Cap for neutral (easier stops)
   if( (spaceThrust.kineticCCW > -kineticNeutralLevel) && (spaceThrust.kineticCCW < kineticNeutralLevel) ) {
    spaceThrust.kineticCCW = 0;
   }


  //Compute space thrust
  spaceThrust.CW = spaceThrust.kineticCW / (kineticMaxLevel / 100);
  spaceThrust.CCW = spaceThrust.kineticCCW / (kineticMaxLevel / 100);
  
  return spaceThrust;
}

//Apply max limit value on thruster
//Usefull when a thuster is less effective
//in one direction than other (reverse/forward)
struct thrustLevel applyMaxLimits (struct thrustLevel thrust, int forwardMaxThrust, int reverseMaxThrust){
   //Do we exceed max authorized power?
   //If so, need to make proportionnal value and cap it
   //For CW motor
   if( (thrust.CW > 0) && (thrust.CW > forwardMaxThrust) ){
     thrust.CW = thrust.CW * (forwardMaxThrust / 100.0);
   } else  if( (thrust.CW < 0) && (thrust.CW < -reverseMaxThrust) ){
     thrust.CW = thrust.CW * (reverseMaxThrust / 100.0);
   }
   //For CCW motor
   if( (thrust.CCW > 0) && (thrust.CCW > forwardMaxThrust) ){
     thrust.CCW = thrust.CCW * (forwardMaxThrust / 100.0);
   } else  if( (thrust.CCW < 0) && (thrust.CCW < -reverseMaxThrust) ){
     thrust.CCW = thrust.CCW * (reverseMaxThrust / 100.0);
   }

   return thrust;
}

//Apply thrust values to thruster
//Use ESC (aka servo) to drive triphase engine
void driveThrusters (struct thrustLevel thrustX, struct thrustLevel thrustY, struct thrustLevel thrustZ){
  xCW.writeMicroseconds(map(thrustX.CW,-100, 100, 1000, 2000));
  xCCW.writeMicroseconds(map(thrustX.CCW,-100, 100, 1000, 2000));
  yCW.writeMicroseconds(map(thrustY.CW,-100, 100, 1000, 2000));
  yCCW.writeMicroseconds(map(thrustY.CCW,-100, 100, 1000, 2000));
  zCW.writeMicroseconds(map(thrustZ.CW,-100, 100, 1000, 2000));
  zCCW.writeMicroseconds(map(thrustZ.CCW,-100, 100, 1000, 2000));
}
  
  
//debug function
//all structs are declared wide.
void printValues (){

  Serial.print("translate X=");
  Serial.print(translation.X);
  Serial.print(" Y=");
  Serial.print(translation.Y);
  Serial.print(" Z=");
  Serial.print(translation.Z);
  
  Serial.print("    rotate X=");
  Serial.print(rotation.X);
  Serial.print(" Y=");
  Serial.print(rotation.Y);
  Serial.print(" Z=");
  Serial.println(rotation.Z);


  //Per thruster power value
  Serial.print("Thruster power X(CW:");
  Serial.print(thrustX.CW);
  Serial.print(" CCW:");
  Serial.print(thrustX.CCW);
  Serial.print(" Kinetic CW:");
  Serial.print(thrustX.kineticCW);
  Serial.print(" CCW:");
  Serial.print(thrustX.kineticCCW);
  Serial.print(") Y(CW:");
  Serial.print(thrustY.CW);
  Serial.print(" CCW:");
  Serial.print(thrustY.CCW);
  Serial.print(" Kinetic CW:");
  Serial.print(thrustY.kineticCW);
  Serial.print(" CCW:");
  Serial.print(thrustY.kineticCCW);
  Serial.print(") Z(CW:");
  Serial.print(thrustZ.CW);
  Serial.print(" CCW:");
  Serial.print(thrustZ.CCW);
  Serial.print(" Kinetic CW:");
  Serial.print(thrustZ.kineticCW);
  Serial.print(" CCW:");
  Serial.print(thrustZ.kineticCCW);
  Serial.println(")");
  }


void loop() {

  //MISSING
  //SpaceMode button switch
  //Armed/Disarmed switch
  //Status light
  
  // read joystock position:
  translateX = analogRead(translateXpin);
  translateY = analogRead(translateYpin);
  translateZ = analogRead(translateZpin);
  
  rotateX = analogRead(rotateXpin);
  rotateY = analogRead(rotateYpin);
  rotateZ = analogRead(rotateZpin);
  
  // stop the program for <sensorValue> milliseconds:
  delay(200);
  
 
  //Convert raw joystick values to percent
  //For translation
  #ifdef inverseTranslateX
  translation.X = joystickToPercent(translateX, true);
  #else
  translation.X = joystickToPercent(translateX);
  #endif
  #ifdef inverseTranslateY
  translation.Y = joystickToPercent(translateY, true);
  #else
 translation.Y = joystickToPercent(translateY);
  #endif
  #ifdef inverseTranslateZ
  translation.Z = joystickToPercent(translateZ, true);
  #else
  translation.Z = joystickToPercent(translateZ);
  #endif

  //For rotation
  #ifdef inverseRotateX
  rotation.X = joystickToPercent(rotateX, true);
  #else
  rotation.X = joystickToPercent(rotateX);
  #endif
  #ifdef inverseRotateY
  rotation.Y = joystickToPercent(rotateY, true);
  #else
  rotation.Y = joystickToPercent(rotateY);
  #endif
  #ifdef inverseRotateZ
  rotation.Z = joystickToPercent(rotateZ, true);
  #else
  rotation.Z = joystickToPercent(rotateZ);
  #endif

  //Convert PercentJoystick to real thrust
  thrustX = percentToThrust(thrustX, translation.X, rotation.X);
  thrustY = percentToThrust(thrustY, translation.Y, rotation.Y);
  thrustZ = percentToThrust(thrustZ, translation.Z, rotation.Z);

  //Space mode. Each action needs to be reciproced
  if ( spaceMode == true ){
    thrustX = spaceKineticThrust(thrustX);
    thrustY = spaceKineticThrust(thrustY);
    thrustZ = spaceKineticThrust(thrustZ);
  }

  //Check and apply max limit power
  thrustX = applyMaxLimits(thrustX, thrusterXForwardMax, thrusterXReverseMax);
  thrustY = applyMaxLimits(thrustY, thrusterYForwardMax, thrusterYReverseMax);
  thrustZ = applyMaxLimits(thrustZ, thrusterZForwardMax, thrusterZReverseMax);

  //Drive thruster
  driveThrusters(thrustX, thrustY, thrustZ);


  //Debug print
  printValues();
  

}
