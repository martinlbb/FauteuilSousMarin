#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

      // variables to hold the parsed data
char messageFromPC[numChars] = {0};
int integerFromPC = 0;
float floatFromPC = 0.0;

boolean newData = false;


void setup() {
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Serial receiver");
  display.display();

  delay(500);
}

//============

void loop() {
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
        // this temporary copy is necessary to protect the original data
        //   because strtok() used in parseData() replaces the commas with \0
        display.clearDisplay();
        display.setCursor(0, 0);
        //display.println(tempChars);
        parseData();
        display.display();
        //showParsedData();
        newData = false;
    }
  delay(100);
}

//============

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '$';
    char endMarker = '!';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

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
      //Go mode for shared joystick
      //G:translateX%:translateY%:translateZ%:rotateX%:rotateY%:rotateZ%!
      case 'G':
      case 'g':
        display.print("GO T:");
        //Recover Translate X value
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Translate Y
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Translate Z    
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" R:");
        //Rotate X
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Rotate Y
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Rotate Z
        strtokIndx = strtok(NULL, ":"); 
        display.println(atoi(strtokIndx));
        break;
        
      //Autoritative Go mode for joysticj
      //G:translateX%:translateY%:translateZ%:rotateX%:rotateY%:rotateZ%!
      case 'H':
      case 'h':
        display.print("FORCE H GO T:");
        //Recover Translate X value
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Translate Y
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Translate Z    
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" R:");
        //Rotate X
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Rotate Y
        strtokIndx = strtok(NULL, ":"); 
        display.print(atoi(strtokIndx));
        display.print(" ");
        //Rotate Z
        strtokIndx = strtok(NULL, ":"); 
        display.println(atoi(strtokIndx));
        break;

      //Space mode
      //$S!
      case 'M':
      case 'm':              
        //Switch which mode we want to use
        strtokIndx = strtok(NULL, ":");
        switch(strtokIndx[0]){
          //Dive mode
          //$M:D!
          case 'D':
          case 'd':
            //int spaceMode = false;
            display.print("Dive Mode");
          break;
          
          //Space mode
          //$S!
          case 'S':
          case 's':
            //int spaceMode = false;
            display.print("Space Mode");
          break;
        }
        break;
        
      //Save parameters
      case 'S':
      case 's':
        display.print("Saving");
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
                 display.print("T X=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T Y=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T Z=");
                 display.print(atoi(strtokIndx));
              }
            } else if (strcmp(strtokIndx, "R")==0){
              //We are modifying Rotate joystick
              strtokIndx = strtok(NULL, ":");
              if(strcmp(strtokIndx, "X")==0){
                 //X axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("R X=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("R Y=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("R Z=");
                 display.print(atoi(strtokIndx));
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
                 display.print("T F X=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T F Y=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T F Z=");
                 display.print(atoi(strtokIndx));
              }
            } else if (strcmp(strtokIndx, "R")==0){
              //We are modifying Rotate joystick
              strtokIndx = strtok(NULL, ":");
              if(strcmp(strtokIndx, "X")==0){
                 //X axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T R X=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Y")==0){
                 //Y axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T R Y=");
                 display.print(atoi(strtokIndx));
              } else if(strcmp(strtokIndx, "Z")==0){
                 //Z axis
                 strtokIndx = strtok(NULL, ":");
                 display.print("T R Z=");
                 display.print(atoi(strtokIndx));
              }
            }
            break;
          //Neutral % value
          case 'N':
          case 'n':
            strtokIndx = strtok(NULL, ":");
            display.print("NEUTRAL %=");
            display.print(atoi(strtokIndx));
            break;
          //Kinetic % value
          case 'K':
          case 'k':
            strtokIndx = strtok(NULL, ":");
            display.print("KINE %=");
            display.print(atoi(strtokIndx));
            break;
        }
        break;
    }




}


void showParsedData() {
}

/*
//============

void parseData() {      // split the data into its parts

    char * strtokIndx; // this is used by strtok() as an index

    switch (strtok(tempChars,":")){
      case 'G':
      case 'g':
        //Go mode for shared joystick
        int translateX = atoi(strtok(NULL,":"));
        int translateY = atoi(strtok(NULL,":"));
        int translateZ = atoi(strtok(NULL,":"));
        int rotateX = atoi(strtok(NULL,":"));
        int rotateY = atoi(strtok(NULL,":"));
        int rotateZ = atoi(strtok(NULL,":"));
        break;
      case 'H':
      case 'h':
        //Autoritative Go mode for joysticj
        int translateX = atoi(strtok(NULL,":"));
        int translateY = atoi(strtok(NULL,":"));
        int translateZ = atoi(strtok(NULL,":"));
        int rotateX = atoi(strtok(NULL,":"));
        int rotateY = atoi(strtok(NULL,":"));
        int rotateZ = atoi(strtok(NULL,":"));
        break;
      case 'S':
      case 's':
        //Space mode
        int spaceMode = true;
        break;
      case 'D':
      case 'd':
        //Dive mode
        int spaceMode = false;
        break;
      case 'P':
      case 'p':                
        //parameter mode
        switch (strtok(NULL,":")){
          case 'I':
          case 'i':
            //Inverse axis orientation
            char mode = strtok(NULL,":");
            char axis = strtok(NULL,":");
            bool inverse = atoi(strtok(NULL,":"));
            break;
          case 'N':
          case 'n':
            //Set neutral threshold
            int neutral = atoi(strtok(NULL,":"));
            break;
          case 'T':
          case 't':
            //Set thruster max percent   
            char orientation = strtok(NULL,":");
            char axis = strtok(NULL,":");
            int percent = atoi(strtok(NULL,":"));
            break;
          case 'K':
          case 'k':
            //Set kinetic ignore value 
            int percent = atoi(strtok(NULL,":"));
          default:
            //No command received, ignore it.
            break;
        }
        default:
          //No command received, ignore it.
          break;
    }
}

*/
