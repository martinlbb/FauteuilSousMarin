
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

#define TFT_DC 40
#define TFT_CS 41
//software SPI for TFT
#define TFT_MOSI 42
#define TFT_CLK 43
#define TFT_MISO 44

#define CS_PIN 44

#define TS_MINX 300
#define TS_MINY 300
#define TS_MAXX 3900
#define TS_MAXY 3900

#define DIVE true
#define SPACE false

#define XPLUST 1
#define XMINT 2
#define YPLUST 3
#define YMINT 4
#define ZPLUST 5
#define ZMINT 6
#define XPLUSR 7
#define XMINR 8
#define YPLUSR 9
#define YMINR 10
#define ZPLUSR 11
#define ZMINR 12

//hardware SPI for TS
//#define TFT_MOSI 51
//#define TFT_CLK 52
//#define TFT_MISO 50

XPT2046_Touchscreen ts(CS_PIN);

//XPT2046_Touchscreen ts(CS_PIN);  // Param 2 - NULL - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, 255);  // Param 2 - 255 - No interrupts
//XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);  // Param 2 - Touch IRQ Pin - interrupt enabled polling

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK);

bool armed=false;
bool mode=DIVE;
short axis=0;
short speed=0;
short oldSpeed=0;

void setup() {
  
  Serial.begin(115200);
  Serial1.begin(115200);
  tft.begin(16000000);
  tft.setRotation(1);
  tft.fillScreen(ILI9341_NAVY);
  ts.begin();
  //ts.setRotation(1);
  while (!Serial && (millis() <= 1000));
  drawMenu();

  
}

boolean wastouched = true;

void loop() {
  int x, y;

  boolean istouched = ts.touched();
  if (istouched) {
    TS_Point p = ts.getPoint();
    //Convert to real x,y coordinate
    //x = map(p.x,300,3900,0,320);
    //y = map(p.y,300,3900,0,240);
    x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());


/*
    //Map armed function
    if((x >= 178)&&(x<=318)&&(y>=3)&&(y<=46)){
      if (armed == true) armed = false; else armed = true;
      drawArmed(armed);
    }
    //Map dive/space mode
    if((x >= 3)&&(x<=146)&&(y>=3)&&(y<=46)){
      if (mode == DIVE) mode = SPACE; else mode = DIVE;
      drawMode(mode);
    }
    */
    //Map dive/space mode
    if((x >= 1)&&(x<=74)&&(y>=1)&&(y<=36)){
      if (mode == DIVE) mode = SPACE; else mode = DIVE;
      drawMode(mode);
    }
    //Map armed function
    if((x >= 1)&&(x<=74)&&(y>=38)&&(y<=75)){
      if (armed == true) armed = false; else armed = true;
      drawArmed(armed);
    }
    //Map speed 
    for(int i=1;i<=9;i++){
      if((x >= (27*i)+50)&&(x<=(27*i)+77)&&(y>=12)&&(y<=62)){
        if(speed != i)oldSpeed=speed;
        drawSpeed(i,true);
        speed=i;
      }
    }
    //Translation
    //map X+ T
    if((x >= 107)&&(x<=157)&&(y>=158)&&(y<=238)){
      axis = XPLUST; 
      drawPadElement('X', true, 'T', 1);
    }
    //map X- T
    if((x >= 3)&&(x<=53)&&(y>=158)&&(y<=238)){
      axis = XMINT; 
      drawPadElement('X', false, 'T', 1);
    }
    //map Y+ T
    if((x >= 55)&&(x<=105)&&(y>=76)&&(y<=156)){
      axis = YPLUST; 
      drawPadElement('Y', true, 'T', 1);
    }
    //map Y- T
    if((x >= 55)&&(x<=105)&&(y>=158)&&(y<=238)){
      axis = YMINT; 
      drawPadElement('Y', false, 'T', 1);
    }
    //map Z+ T
    if((x >= 3)&&(x<=53)&&(y>=76)&&(y<=156)){
      axis = ZPLUST; 
      drawPadElement('Z', true, 'T', 1);
    }
    //map Z- T
    if((x >= 107)&&(x<=157)&&(y>=76)&&(y<=156)){
      axis = ZMINT; 
      drawPadElement('Z', false, 'T', 1);
    }
    //Rotation
    //map X+ R
    if((x >= 107+160)&&(x<=157+160)&&(y>=158)&&(y<=238)){
      axis = XPLUSR; 
      drawPadElement('X', true, 'R', 1);
    }
    //map X- R
    if((x >= 3+160)&&(x<=53+160)&&(y>=158)&&(y<=238)){
      axis = XMINR; 
      drawPadElement('X', false, 'R', 1);
    }
    //map Y+ R
    if((x >= 55+160)&&(x<=105+160)&&(y>=76)&&(y<=156)){
      axis = YPLUSR; 
      drawPadElement('Y', true, 'R', 1);
    }
    //map Y- R
    if((x >= 55+160)&&(x<=105+160)&&(y>=158)&&(y<=238)){
      axis = YMINR; 
      drawPadElement('Y', false, 'R', 1);
    }
    //map Z+ R
    if((x >= 3+160)&&(x<=53+160)&&(y>=76)&&(y<=156)){
      axis = ZPLUSR; 
      drawPadElement('Z', true, 'R', 1);
    }
    //map Z- R
    if((x >= 107+160)&&(x<=157+160)&&(y>=76)&&(y<=156)){
      axis = ZMINR; 
      drawPadElement('Z', false, 'R', 1);
    }
  } else {
      switch(axis){
        case XPLUST:
          drawPadElement('X', true, 'T', 0);
          break;
        case XMINT:
          drawPadElement('X', false, 'T', 0);
          break;
        case YPLUST:
          drawPadElement('Y', true, 'T', 0);
          break;
        case YMINT:
          drawPadElement('Y', false, 'T', 0);
          break;
        case ZPLUST:
          drawPadElement('Z', true, 'T', 0);
          break;
        case ZMINT:
          drawPadElement('Z', false, 'T', 0);
          break;
        case XPLUSR:
          drawPadElement('X', true, 'R', 0);
          break;
        case XMINR:
          drawPadElement('X', false, 'R', 0);
          break;
        case YPLUSR:
          drawPadElement('Y', true, 'R', 0);
          break;
        case YMINR:
          drawPadElement('Y', false, 'R', 0);
          break;
        case ZPLUSR:
          drawPadElement('Z', true, 'R', 0);
          break;
        case ZMINR:
          drawPadElement('Z', false, 'R', 0);
          break;
      }
      axis=0;
      //Refresh speed if needed
      if(oldSpeed != 0){
        drawSpeed(oldSpeed,false);
        oldSpeed = 0;
      }
  }
  //Recopy any serial info
  if (Serial.available() > 0) {
    Serial1.write(Serial.read());
  }
  /*
   * Should not be used, as we are only in master mode
  if (Serial1.available() > 0) {
    Serial.write(Serial1.read());
  }
  */
  //delay(50);
}


/*
void drawArmed(bool enable){
  if(enable){
    Serial.println("Armed");
    //Armed
    tft.drawRoundRect(178,3,140, 43, 4, ILI9341_WHITE);
    tft.fillRoundRect(178,3,140, 43, 4, ILI9341_RED);
    tft.setCursor(205,14);
    tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    tft.setTextSize(3);
    tft.print("ARMED");
  }else {
    Serial.println("Disarmed");
    //Disarmed
    tft.drawRoundRect(178,3,140, 43, 4, ILI9341_WHITE);
    tft.fillRoundRect(178,3,140, 43, 4, ILI9341_GREEN); 
    tft.setCursor(203,18);
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
    tft.setTextSize(2);
    tft.print("DISARMED");
  }
}

void drawMode(bool mode){
  if(mode == DIVE){
    //Currently in Dive mode
    tft.drawRoundRect(3,3,140, 43, 4, ILI9341_WHITE);
    tft.fillRoundRect(3,3,140, 43, 4, ILI9341_BLUE);
    tft.setCursor(38,15);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    tft.setTextSize(3);
    tft.print("DIVE");
  }else{
    //Currently in Space mode
    tft.drawRoundRect(3,3,140, 43, 4, ILI9341_WHITE);
    tft.fillRoundRect(3,3,140, 43, 4, ILI9341_LIGHTGREY);
    tft.setCursor(31,15);
    tft.setTextColor(ILI9341_WHITE, ILI9341_LIGHTGREY);
    tft.setTextSize(3);
    tft.print("SPACE");
  }
}

//Temp version for speed on screen
void drawMode(bool mode){
  if(mode == DIVE){
    //Currently in Dive mode
    tft.drawRoundRect(3,3,70, 43, 4, ILI9341_WHITE);
    tft.fillRoundRect(3,3,70, 43, 4, ILI9341_BLUE);
    tft.setCursor(17,15);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    tft.setTextSize(2);
    tft.print("DIVE");
    Serial1.println("$M:D!");
  }else{
    //Currently in Space mode
    tft.drawRoundRect(3,3,70, 43, 4, ILI9341_WHITE);
    tft.fillRoundRect(3,3,70, 43, 4, ILI9341_LIGHTGREY);
    tft.setCursor(9,15);
    tft.setTextColor(ILI9341_WHITE, ILI9341_LIGHTGREY);
    tft.setTextSize(2);
    tft.print("SPACE");
    Serial1.println("$M:S!");
  }
}

*/

void drawMode(bool mode){
  if(mode == DIVE){
    //Currently in Dive mode
    tft.drawRoundRect(1,1,70, 36, 4, ILI9341_WHITE);
    tft.fillRoundRect(1,1,70, 36, 4, ILI9341_BLUE);
    tft.setCursor(15,12);
    tft.setTextColor(ILI9341_WHITE, ILI9341_BLUE);
    tft.setTextSize(2);
    tft.print("DIVE");
    Serial1.println("$M:D!");
  }else{
    //Currently in Space mode
    tft.drawRoundRect(1,1,70, 36, 4, ILI9341_WHITE);
    tft.fillRoundRect(1,1,70, 36, 4, ILI9341_LIGHTGREY);
    tft.setCursor(8,13);
    tft.setTextColor(ILI9341_WHITE, ILI9341_LIGHTGREY);
    tft.setTextSize(2);
    tft.print("SPACE");
    Serial1.println("$M:S!");
  }
}

void drawArmed(bool enable){
  if(enable){
    Serial.println("Armed");
    //Armed
    tft.drawRoundRect(1,38,72, 36, 4, ILI9341_WHITE);
    tft.fillRoundRect(1,38,72, 36, 4, ILI9341_RED);
    tft.setCursor(8,49);
    tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    tft.setTextSize(2);
    tft.print("ARMED");
    Serial1.println("$M:A!");
  }else {
    Serial.println("Disarmed");
    //Disarmed
    tft.drawRoundRect(1,38,72, 36, 4, ILI9341_WHITE);
    tft.fillRoundRect(1,38,72, 36, 4, ILI9341_GREEN); 
    tft.setCursor(1,49);
    tft.setTextColor(ILI9341_BLACK, ILI9341_GREEN);
    tft.setTextSize(2);
    tft.print("DISARM");
    Serial1.println("$M:X!");
  }
}

void drawPadElement(char axis, bool positive, char vector, bool push){
  int x=0;
  if(vector == 'R')x=160;

  switch(axis){
    case 'X':
      if(positive){
        if(push){
          //User just clicked on the button
          tft.fillRoundRect(107+x,158,50,80,4,ILI9341_RED); //X+
          tft.drawRoundRect(107+x,158,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //X+
          tft.fillTriangle(115+x,189, 115+x,222, 147+x,206, ILI9341_DARKGREY);         //X+
          tft.drawTriangle(115+x,189, 115+x,222, 147+x,206, ILI9341_WHITE );//X+
          //Send order
          if(vector == 'T'){
            Serial1.print("$G:+");
            Serial1.print(speed*10);
            Serial1.println(":0:0:0:0:0!");
          }else{
            Serial1.print("$G:0:0:0:+");
            Serial1.print(speed*10);
            Serial1.println(":0:0!");
          }
        }else{
          tft.fillRoundRect(107+x,158,50,80,4,ILI9341_NAVY); //X+
          tft.drawRoundRect(107+x,158,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //X+          
          tft.fillTriangle(115+x,189, 115+x,222, 147+x,206, ILI9341_RED);         //X+
          tft.drawTriangle(115+x,189, 115+x,222, 147+x,206, ILI9341_GREENYELLOW );//X+          
        }
      } else {
        if(push){
          //User just clicked on the button  
          tft.fillRoundRect(3+x,158,50,80,4,ILI9341_RED); //X-      
          tft.drawRoundRect(3+x,158,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //X-
          tft.fillTriangle(43+x,190, 43+x,222, 11+x,205, ILI9341_DARKGREY);            //X-
          tft.drawTriangle(43+x,190, 43+x,222, 11+x,205, ILI9341_WHITE );   //X-
          //Send order
          if(vector == 'T'){
            Serial1.print("$G:-");
            Serial1.print(speed*10);
            Serial1.println(":0:0:0:0:0!");
          }else{
            Serial1.print("$G:0:0:0:-");
            Serial1.print(speed*10);
            Serial1.println(":0:0!");
          }
        }else{
          tft.fillRoundRect(3+x,158,50,80,4,ILI9341_NAVY); //X-  
          tft.drawRoundRect(3+x,158,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //X-
          tft.fillTriangle(43+x,190, 43+x,222, 11+x,205, ILI9341_RED);            //X-
          tft.drawTriangle(43+x,190, 43+x,222, 11+x,205, ILI9341_GREENYELLOW );   //X-
        }
      }
      break;
    case 'Y':
      if(positive){
        if(push){
          //User just clicked on the button
          tft.fillRoundRect(55+x,76,50,80,4,ILI9341_GREEN); //Y+
          tft.drawRoundRect(55+x,76,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Y+
          tft.fillTriangle(63+x,130, 95+x,130, 79+x,98, ILI9341_DARKGREY);          //Y+
          tft.drawTriangle(63+x,130, 95+x,130, 79+x,98, ILI9341_WHITE );   //Y+
          //Send order
          if(vector == 'T'){
            Serial1.print("$G:00:+");
            Serial1.print(speed*10);
            Serial1.println(":0:0:0:0!");
          }else{
            Serial1.print("$G:0:0:0:0:+");
            Serial1.print(speed*10);
            Serial1.println(":0!");
          }
        }else{
          tft.fillRoundRect(55+x,76,50,80,4,ILI9341_NAVY); //Y+
          tft.drawRoundRect(55+x,76,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Y+
          tft.fillTriangle(63+x,130, 95+x,130, 79+x,98, ILI9341_GREEN);          //Y+
          tft.drawTriangle(63+x,130, 95+x,130, 79+x,98, ILI9341_GREENYELLOW );   //Y+
        }
      } else {
        if(push){
          //User just clicked on the button
          tft.fillRoundRect(55+x,158,50,80,4,ILI9341_GREEN); //Y-
          tft.drawRoundRect(55+x,158,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Y-
          tft.fillTriangle(63+x,190, 96+x,190, 79+x,222, ILI9341_DARKGREY);          //Y-
          tft.drawTriangle(63+x,190, 96+x,190, 79+x,222, ILI9341_WHITE );   //Y-
          //Send order
          if(vector == 'T'){
            Serial1.print("$G:00:-");
            Serial1.print(speed*10);
            Serial1.println(":0:0:0:0!");
          }else{
            Serial1.print("$G:0:0:0:0:-");
            Serial1.print(speed*10);
            Serial1.println(":0!");
          }
        }else{
          tft.fillRoundRect(55+x,158,50,80,4,ILI9341_NAVY); //Y-
          tft.drawRoundRect(55+x,158,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Y-
          tft.fillTriangle(63+x,190, 96+x,190, 79+x,222, ILI9341_GREEN);          //Y-
          tft.drawTriangle(63+x,190, 96+x,190, 79+x,222, ILI9341_GREENYELLOW );   //Y-
        }
      }
      break;  
    case 'Z':
      if(positive){
        if(push){
          //User just clicked on the button
          tft.fillRoundRect(3+x,76,50,80,4,ILI9341_BLUE); //Z+
          tft.drawRoundRect(3+x,76,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Z+
          tft.fillTriangle(11+x,132, 45+x,132, 28+x,77, ILI9341_DARKGREY);           //Z+
          tft.drawTriangle(11+x,132, 45+x,132, 28+x,77, ILI9341_WHITE );   //Z+
          //Send order
          if(vector == 'T'){
            Serial1.print("$G:00:00:+");
            Serial1.print(speed*10);
            Serial1.println(":0:0:0!");
          }else{
            Serial1.print("$G:0:0:0:0:0:+");
            Serial1.print(speed*10);
            Serial1.println("!");
          }
        }else{
          tft.fillRoundRect(3+x,76,50,80,4,ILI9341_NAVY); //Z+
          tft.drawRoundRect(3+x,76,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Z+
          tft.fillTriangle(11+x,132, 45+x,132, 28+x,77, ILI9341_BLUE);           //Z+
          tft.drawTriangle(11+x,132, 45+x,132, 28+x,77, ILI9341_GREENYELLOW );   //Z+
        }
      } else {
        if(push){
          //User just clicked on the button
          tft.fillRoundRect(107+x,76,50,80,4,ILI9341_BLUE); //Z-
          tft.drawRoundRect(107+x,76,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Z-
          tft.fillTriangle(115+x,100, 149+x,100, 131+x,153, ILI9341_DARKGREY);       //Z-
          tft.drawTriangle(115+x,100, 149+x,100, 131+x,153, ILI9341_WHITE );//Z-
          //Send order
          if(vector == 'T'){
            Serial1.print("$G:00:00:-");
            Serial1.print(speed*10);
            Serial1.println(":0:0:0!");
          }else{
            Serial1.print("$G:0:0:0:0:0:-");
            Serial1.print(speed*10);
            Serial1.println("!");
          }
        }else{
          tft.fillRoundRect(107+x,76,50,80,4,ILI9341_NAVY); //Z-
          tft.drawRoundRect(107+x,76,50,80,4,(vector == 'R' ? ILI9341_ORANGE : ILI9341_YELLOW)); //Z-
          tft.fillTriangle(115+x,100, 149+x,100, 131+x,153, ILI9341_BLUE);       //Z-
          tft.drawTriangle(115+x,100, 149+x,100, 131+x,153, ILI9341_GREENYELLOW );//Z-
        }
      }
      break;     
  }
}

void drawSpeed(short speed, bool push){

  if(push == true){
    tft.fillRect((27*speed)+50,12,27,50,ILI9341_RED);
    //tft.drawRect((27*speed)+50,12,27,50,ILI9341_RED);
    tft.setCursor((27*speed)+52,30);
    tft.setTextColor(ILI9341_WHITE, ILI9341_RED);
    tft.setTextSize(2);
    tft.print(speed*10);
  }else{
    tft.fillRect((27*speed)+50,12,27,50,ILI9341_DARKCYAN);
    tft.drawRect((27*speed)+50,12,27,50,ILI9341_WHITE);
    tft.setCursor((27*speed)+52,30);
    tft.setTextColor(ILI9341_WHITE, ILI9341_DARKCYAN);
    tft.setTextSize(2);
    tft.print(speed*10);
  }
}

void drawMenu(){
  
  drawMode('D');
  drawArmed(armed);

  for(int i=1;i<=9;i++) { drawSpeed(i, false);}


  //for(int i=1;i<=9;i++) { drawSpeed(i, true);}


  //Translation
  drawPadElement('X', true, 'T', 0);
  drawPadElement('X', false, 'T', 0);
  drawPadElement('Y', true, 'T', 0);
  drawPadElement('Y', false, 'T', 0);
  drawPadElement('Z', true, 'T', 0);
  drawPadElement('Z', false, 'T', 0);
  
  //Rotation
  drawPadElement('X', true, 'R', 0);
  drawPadElement('X', false, 'R', 0);
  drawPadElement('Y', true, 'R', 0);
  drawPadElement('Y', false, 'R', 0);
  drawPadElement('Z', true, 'R', 0);
  drawPadElement('Z', false, 'R', 0);

}
