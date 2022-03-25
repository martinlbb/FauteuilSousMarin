/*
 * Fauteuil Spatial 
 * 
 * Odysseus 3.1
 */

#include <Servo.h>
#include <EEPROM.h>


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

//MAX485 location
//RE is 22 GND
//DE is 23 GND
#define RE 22
#define DE 23

struct parameters {
  unsigned long magic=false;
  byte struct_version;
  //Enable to inverse (swap) current axis value
  bool inverseTranslateX;
  bool inverseTranslateY;
  bool inverseTranslateZ;
  bool inverseRotateX;
  bool inverseRotateY;
  bool inverseRotateZ;
  //Neutral ignore
  //Define how much percentage must be ignored
  //around neutral
  //Axis value is usually -100%<->0%<->+100%
  //Then everyting arrounf -neutral%<->0%<->+neutral%
  //will be ignored. Default is 8%
  //Value must be between 0 and 20 maximum.
  int neutralPercent;
  //Space mode selector
  bool spaceMode;
  //Neutral limit to avoid boucing
  //usually 10%
  int  kineticNeutralLevel;
  //Kinetic limit
  //Usually defined at 1000 (100% * 10)
  int kineticMaxLevel;
  //Thuster max power
  //define max percentage of power
  //this thruster can handle (usually
  // up to 100%)
  //Allow limiting Forward power to
  //compensate for Reverse power
  //ie: FW=70%, RV=100%
  int thrusterXForwardMax;
  int thrusterXReverseMax;
  int thrusterYForwardMax;
  int thrusterYReverseMax;
  int thrusterZForwardMax;
  int thrusterZReverseMax;
  //Debug flag for output on USB/serial
  bool debug;
};

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

//Magic number for Eeprom
static const unsigned long STRUCT_MAGIC = 78851203;
static const byte STRUCT_VERSION = 1;

//To deal with serial orders
const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing
boolean newData = false;

//Global wide variable with parameters

bool armed = false;
parameters p;
//ESC declaration
Servo xCW, xCCW, yCW, yCCW, zCW, zCCW;


//Per axis definition 
struct thrustLevel thrustX;
struct thrustLevel thrustY;
struct thrustLevel thrustZ;

//Per joystick definition 
struct joystickPercent translation;
struct joystickPercent rotation;

void setup() {
  //Initialize MAS485
  pinMode(DE, OUTPUT);
  pinMode(RE, OUTPUT);
  digitalWrite(DE, LOW);
  digitalWrite(RE, LOW);
  
  
  //Initialize serial debug link
  Serial.begin(115200);
  while (!Serial) {
     ; // wait for serial port to connect. Needed for native USB
  }
  Serial.println("SpaceChair ready");
  //RS485 serial link 
  Serial1.begin(115200);

  //Load parameters
  loadEEPROM();

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
  if ((value > -p.neutralPercent) && (value < p.neutralPercent) ) {
    value = 0;
  } else {
    //Recompute value to ignore neutral while driving motor smootlhy
    if ( value > 0){
      //For positive value
      value = (value * (1 + (p.neutralPercent * 0.01))) - p.neutralPercent;
    } else {
      value = (value * (1 + (p.neutralPercent * 0.01))) + p.neutralPercent;
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
   if( (spaceThrust.kineticCW > 0 ) && (spaceThrust.kineticCW > p.kineticMaxLevel) ) {
    spaceThrust.kineticCW = p.kineticMaxLevel;
   } else if( (spaceThrust.kineticCW < 0) && (spaceThrust.kineticCW < -p.kineticMaxLevel) ) {
    spaceThrust.kineticCW = -p.kineticMaxLevel;
   }
   //Cap for neutral (easier stops)
   if( (spaceThrust.kineticCW > -p.kineticNeutralLevel) && (spaceThrust.kineticCW < p.kineticNeutralLevel) ) {
    spaceThrust.kineticCW = 0;
   }
   //For CCW
   if( (spaceThrust.kineticCCW > 0 ) && (spaceThrust.kineticCCW > p.kineticMaxLevel) ) {
    spaceThrust.kineticCCW = p.kineticMaxLevel;
   } else if( (spaceThrust.kineticCCW < 0) && (spaceThrust.kineticCCW < -p.kineticMaxLevel) ) {
    spaceThrust.kineticCCW = -p.kineticMaxLevel;
   }
   //Cap for neutral (easier stops)
   if( (spaceThrust.kineticCCW > -p.kineticNeutralLevel) && (spaceThrust.kineticCCW < p.kineticNeutralLevel) ) {
    spaceThrust.kineticCCW = 0;
   }


  //Compute space thrust
  spaceThrust.CW = spaceThrust.kineticCW / (p.kineticMaxLevel / 100);
  spaceThrust.CCW = spaceThrust.kineticCCW / (p.kineticMaxLevel / 100);
  
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
  //We have ARMED thrusters
  if (armed == true){
    xCW.writeMicroseconds(map(thrustX.CW,-100, 100, 1000, 2000));
    xCCW.writeMicroseconds(map(thrustX.CCW,-100, 100, 1000, 2000));
    yCW.writeMicroseconds(map(thrustY.CW,-100, 100, 1000, 2000));
    yCCW.writeMicroseconds(map(thrustY.CCW,-100, 100, 1000, 2000));
    zCW.writeMicroseconds(map(thrustZ.CW,-100, 100, 1000, 2000));
    zCCW.writeMicroseconds(map(thrustZ.CCW,-100, 100, 1000, 2000));
  } else {
    //Let's disarm thrusters
    //Full OFF on every axis
    xCW.writeMicroseconds(1500);
    xCCW.writeMicroseconds(1500);
    yCW.writeMicroseconds(1500);
    yCCW.writeMicroseconds(1500);
    zCW.writeMicroseconds(1500);
    zCCW.writeMicroseconds(1500);
  }
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

//debug function
//all structs are declared wide.
void printParameters (){
  //Debug
  Serial.print("Debug =");
  Serial.println(p.debug);

  //Space mode
  Serial.print("Space mode =");
  Serial.println(p.spaceMode);
  
  //Translate
  Serial.print("Inversed Translate X=");
  Serial.println(p.inverseTranslateX);
  Serial.print("Inversed Translate Y=");
  Serial.println(p.inverseTranslateY);
  Serial.print("Inversed Translate Z=");
  Serial.println(p.inverseTranslateZ);

  //Rotate
  Serial.print("Inversed Rotate X=");
  Serial.println(p.inverseRotateX);
  Serial.print("Inversed Rotate Y=");
  Serial.println(p.inverseRotateY);
  Serial.print("Inversed Rotate Z=");
  Serial.println(p.inverseRotateZ);
    
  //Neutral percent
  Serial.print("Neutral ignore%=");
  Serial.println(p.neutralPercent);

  //Kinetic neutral level
  Serial.print("Kinetic neutral ignore%=");
  Serial.println(p.kineticNeutralLevel);

  //Kinetic max level
  Serial.print("Kinetic max level=");
  Serial.println(p.kineticMaxLevel);

  //Thruster max level
  Serial.print("Thruster X FW max%=");
  Serial.println(p.thrusterXForwardMax);
  Serial.print("Thruster X RV max%=");
  Serial.println(p.thrusterXReverseMax); 
     
  Serial.print("Thruster Y FW max%=");
  Serial.println(p.thrusterYForwardMax);
  Serial.print("Thruster Y RV max%=");
  Serial.println(p.thrusterYReverseMax);   
  
  Serial.print("Thruster Z FW max%=");
  Serial.println(p.thrusterZForwardMax);
  Serial.print("Thruster Z RV max%=");
  Serial.println(p.thrusterZReverseMax);   
}


//Load Eeprom content in parameters
void loadEEPROM() {

  // Read eeprom
  EEPROM.get(0, p);

  Serial.println("Reading eeprom");
  
  // No custom data saved
  //Imput default values
  if (p.magic != STRUCT_MAGIC) {
    Serial.println("Empty Eeprom, applying default values");
    //First clear Eeprom
    //for (int i = 0 ; i < EEPROM.length() ; i++) {
    //  EEPROM.write(i, 0);
    //}
    p.inverseTranslateX=false;
    p.inverseTranslateY=false;
    p.inverseTranslateZ=false;
    p.inverseRotateX=false;
    p.inverseRotateY=false;
    p.inverseRotateZ=false;
    p.neutralPercent=8;
    p.spaceMode=false;
    p.kineticNeutralLevel=10;
    p.kineticMaxLevel=1000;
    p.thrusterXForwardMax=75;
    p.thrusterXReverseMax=100;
    p.thrusterYForwardMax=75;
    p.thrusterYReverseMax=100;
    p.thrusterZForwardMax=75;
    p.thrusterZReverseMax=100;
    p.debug=false;

    // Backup data
    saveEEPROM();
    //Proint default values
    printParameters();
  } 

  //Debug values
  if(p.debug==true) printParameters();
}

//Save parameters to eeprom
void saveEEPROM() {
  p.magic = STRUCT_MAGIC;
  p.struct_version =  STRUCT_VERSION;
  EEPROM.put(0, p);
}



void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '$';
    char endMarker = '!';
    char rc;

    while (Serial1.available() > 0 && newData == false) {
        rc = Serial1.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}


void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index
    strtokIndx = strtok(tempChars,":");      // get the first part - parameter

    switch(strtokIndx[0]){
      //Autoritative Go mode for shared joystick
      //G:translateX%:translateY%:translateZ%:rotateX%:rotateY%:rotateZ%!
      case 'G':
      case 'g':
        //Recover Translate X value
        strtokIndx = strtok(NULL, ":"); 
        translation.X = constrain(atoi(strtokIndx),-100,100);
        //Translate Y
        strtokIndx = strtok(NULL, ":"); 
        translation.Y = constrain(atoi(strtokIndx),-100,100);
        //Translate Z    
        strtokIndx = strtok(NULL, ":");
        translation.Z = constrain(atoi(strtokIndx),-100,100);
        //Rotate X
        strtokIndx = strtok(NULL, ":");
        rotation.X = constrain(atoi(strtokIndx),-100,100);
        //Rotate Y
        strtokIndx = strtok(NULL, ":");
        rotation.Y = constrain(atoi(strtokIndx),-100,100);
        //Rotate Z
        strtokIndx = strtok(NULL, ":");
        rotation.Z = constrain(atoi(strtokIndx),-100,100);
        //Debug
        if(p.debug==true){
          Serial.print("MSG: GO translate X=");
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
        }
        break;
        
      //Space mode
      //$M:S!
      case 'M':
      case 'm':              
        //Switch which mode we want to use
        strtokIndx = strtok(NULL, ":");
        switch(strtokIndx[0]){
          //ARMED
          //$M:A!
          case 'A':
          case 'a':
            armed = true;
            //Debug
            if(p.debug==true) Serial.println("MSG: ARMED");
          break;

          //DISARMED
          //$M:X!
          case 'X':
          case 'x':
            armed = false;
            //Debug
            if(p.debug==true) Serial.println("MSG: DISARMED");
          break;

          
          //Dive mode
          //$M:D!
          case 'D':
          case 'd':
            p.spaceMode = false;
            //Debug
            if(p.debug==true) Serial.println("MSG: Dive Mode");
          break;
          
          //Space mode
          //$M:S!
          case 'S':
          case 's':
            //Reset kinetic values to zero 
            thrustX.kineticCW = thrustX.kineticCCW = 0;
            thrustY.kineticCW = thrustY.kineticCCW = 0;
            thrustZ.kineticCW = thrustZ.kineticCCW = 0;
            p.spaceMode = true;
            //Debug
            if(p.debug==true) Serial.println("MSG: Space Mode");
          break;
        }
        break;

      //Save parameters
      case 'D':
      case 'd':
        p.debug=!p.debug;
        if(p.debug==true)Serial.println("MSG: Debug is enabled");
        break;
        
      //Save parameters
      case 'S':
      case 's':
        saveEEPROM();
        if(p.debug==true) Serial.println("MSG: Parameters saved to Eeprom");
        break;
      //Parameter mode  
      case 'P':
      case 'p':                
        //Switch which parameter we want to change
        strtokIndx = strtok(NULL, ":");
        switch(strtokIndx[0]){
          //Inverse joystick axis
          case 'I':
          case 'i':
            strtokIndx = strtok(NULL, ":");
            if(strcmp(strtokIndx, "T")==0){
              //We are modifying Translate joystick
              strtokIndx = strtok(NULL, ":");
              if(strcmp(strtokIndx, "X")==0){
                 //X axis
                 strtokIndx = strtok(NULL, ":");
                 p.inverseTranslateX=constrain(atoi(strtokIndx),0,1);
                 if((p.debug==true)&&(p.inverseTranslateX==true))Serial.println("MSG: Translate X is inversed");
                 if((p.debug==true)&&(p.inverseTranslateX==false))Serial.println("MSG: Translate X is not inversed");
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 p.inverseTranslateY=constrain(atoi(strtokIndx),0,1);
                 if((p.debug==true)&&(p.inverseTranslateY==true))Serial.println("MSG: Translate Y is inversed");
                 if((p.debug==true)&&(p.inverseTranslateY==false))Serial.println("MSG: Translate Y is not inversed");
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 p.inverseTranslateZ=constrain(atoi(strtokIndx),0,1);
                 if((p.debug==true)&&(p.inverseTranslateZ==true))Serial.println("MSG: Translate Z is inversed");
                 if((p.debug==true)&&(p.inverseTranslateZ==false))Serial.println("MSG: Translate Z is not inversed");
              }
            } else if (strcmp(strtokIndx, "R")==0){
              //We are modifying Rotate joystick
              strtokIndx = strtok(NULL, ":");
              if(strcmp(strtokIndx, "X")==0){
                 //X axis
                 strtokIndx = strtok(NULL, ":");
                 p.inverseRotateX=constrain(atoi(strtokIndx),0,1);
                 if((p.debug==true)&&(p.inverseRotateX==true))Serial.println("MSG: Rotate X is inversed");
                 if((p.debug==true)&&(p.inverseRotateX==false))Serial.println("MSG: Rotate X is not inversed");
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 p.inverseRotateY=constrain(atoi(strtokIndx),0,1);
                 if((p.debug==true)&&(p.inverseRotateY==true))Serial.println("MSG: Rotate Y is inversed");
                 if((p.debug==true)&&(p.inverseRotateY==false))Serial.println("MSG: Rotate Y is not inversed");
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 p.inverseRotateZ=constrain(atoi(strtokIndx),0,1);
                 if((p.debug==true)&&(p.inverseRotateZ==true))Serial.println("MSG: Rotate Z is inversed");
                 if((p.debug==true)&&(p.inverseRotateZ==false))Serial.println("MSG: Rotate Z is not inversed");
              }
            }
            break;
          //Thruster max % value
          case 'T':
          case 't':
            strtokIndx = strtok(NULL, ":");
            if(strcmp(strtokIndx, "F")==0){
              //We are in forward mode
              strtokIndx = strtok(NULL, ":");
              if(strcmp(strtokIndx, "X")==0){
                 //X axis
                 strtokIndx = strtok(NULL, ":");
                 p.thrusterXForwardMax=constrain(atoi(strtokIndx),0,100);
                 if(p.debug==true){
                    Serial.print("MSG: Thruster X FW max %:");
                    Serial.println(p.thrusterXForwardMax);
                 }
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 p.thrusterYForwardMax=constrain(atoi(strtokIndx),0,100);
                 if(p.debug==true){
                    Serial.print("MSG: Thruster Y FW max %:");
                    Serial.println(p.thrusterYForwardMax);
                 }
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 p.thrusterZForwardMax=constrain(atoi(strtokIndx),0,100);
                 if(p.debug==true){
                    Serial.print("MSG: Thruster Z FW max %:");
                    Serial.println(p.thrusterZForwardMax);
                 }
              }
            } else if (strcmp(strtokIndx, "R")==0){
              //We are modifying Rotate joystick
              strtokIndx = strtok(NULL, ":");
              if(strcmp(strtokIndx, "X")==0){
                 //X axis
                 strtokIndx = strtok(NULL, ":");
                 p.thrusterXReverseMax=constrain(atoi(strtokIndx),0,100);
                 if(p.debug==true){
                    Serial.print("MSG: Thruster X RV max %:");
                    Serial.println(p.thrusterXReverseMax);
                 }
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 p.thrusterYReverseMax=constrain(atoi(strtokIndx),0,100);
                 if(p.debug==true){
                    Serial.print("MSG: Thruster Y RV max %:");
                    Serial.println(p.thrusterYReverseMax);
                 }
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 p.thrusterZReverseMax=constrain(atoi(strtokIndx),0,100);
                 if(p.debug==true){
                    Serial.print("MSG: Thruster Z RV max %:");
                    Serial.println(p.thrusterZReverseMax);
                 }
              }
            }
            break;
          //Neutral % value
          case 'N':
          case 'n':
            strtokIndx = strtok(NULL, ":");
            p.neutralPercent=(constrain(atoi(strtokIndx),0,100));
            if(p.debug==true){
              Serial.print("MSG: Neutral ignore % :");
              Serial.println(p.neutralPercent);
            }
            break;
          //Kinetic % value
          case 'K':
          case 'k':
            strtokIndx = strtok(NULL, ":");
            p.kineticNeutralLevel=(constrain(atoi(strtokIndx),0,100));
            if(p.debug==true){
              Serial.print("MSG: Kinetic neutral ignore % :");
              Serial.println(p.kineticNeutralLevel);
            }
            break;
        }
        break;
    }
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
  
  //Convert raw joystick values to percent
  //For translation
  translation.X = joystickToPercent(translateX, p.inverseTranslateX);
  translation.Y = joystickToPercent(translateY, p.inverseTranslateY);
  translation.Z = joystickToPercent(translateZ, p.inverseTranslateZ);

  //For rotation
  rotation.X = joystickToPercent(rotateX, p.inverseRotateX);
  rotation.Y = joystickToPercent(rotateY, p.inverseRotateY);
  rotation.Z = joystickToPercent(rotateZ, p.inverseRotateZ);

  //Read serial link for any orders
  recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
        // this temporary copy is necessary to protect the original data
        //   because strtok() used in parseData() replaces the commas with \0
        parseData();
        newData = false;
    }

  //Convert PercentJoystick to real thrust
  thrustX = percentToThrust(thrustX, translation.X, rotation.X);
  thrustY = percentToThrust(thrustY, translation.Y, rotation.Y);
  thrustZ = percentToThrust(thrustZ, translation.Z, rotation.Z);

  //Space mode. Each action needs to be reciproced
  if ( p.spaceMode == true ){
    thrustX = spaceKineticThrust(thrustX);
    thrustY = spaceKineticThrust(thrustY);
    thrustZ = spaceKineticThrust(thrustZ);
  }

  //Check and apply max limit power
  thrustX = applyMaxLimits(thrustX, p.thrusterXForwardMax, p.thrusterXReverseMax);
  thrustY = applyMaxLimits(thrustY, p.thrusterYForwardMax, p.thrusterYReverseMax);
  thrustZ = applyMaxLimits(thrustZ, p.thrusterZForwardMax, p.thrusterZReverseMax);

  //Drive thruster (only if armed)<, within function
  driveThrusters(thrustX, thrustY, thrustZ);
  
  //Debug print
  if(p.debug==true) printValues();


  delay(250);
  

}
