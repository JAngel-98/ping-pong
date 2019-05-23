#include <SPI.h> // f.k. for Arduino-1.5.2
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include<SoftwareSerial.h>

#define MINPRESSURE 200
#define MAXPRESSURE 1000

const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_LEFT = 117, TS_RT = 897, TS_TOP = 76, TS_BOT = 886;

MCUFRIEND_kbv tft; // hard-wired for UNO shields anyway.
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button left, right;
SoftwareSerial Serie2(10, 11); 

int pixel_x, pixel_y; //Touch_getXY() updates global vars
int16_t block;
uint16_t ID, x, y, xB, yB, yButton;
int8_t cX = 1, cY = -1;
bool ball;

// Assign human-readable names to some common 16-bit color values:
#define	BLACK        0x0000
#define	BLUE         0x001F
#define	RED          0xF800
#define	GREEN        0x07E0
#define YELLOW       0xFFE0
#define WHITE        0xFFFF
#define LightGrey    0xC618
#define DarkGrey     0x7BEF
#define Orange       0xFD20

void setup(void) {
  tft.reset();
  ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(0);
  tft.fillScreen(BLACK);
  
  Serie2.begin(9600);  
  Serial.begin(9600);

  block = tft.width() / 6;
  tft.fillScreen(BLACK);
  
  x = 80;
  y = 280; //Barra
  xB = x + 40;
  yB = y - 10; //Pelota
  yButton = 310; //Altura Botones
  
  left.initButton(&tft,  20, yButton, 40, 20, WHITE, BLACK, WHITE, "Left", 1);
  right.initButton(&tft, 240 - 20, yButton, 40, 20, WHITE, BLACK, WHITE, "Right", 1);
  left.drawButton(false);
  right.drawButton(false);
  
  tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  tft.drawCircle(xB, yB, 9, GREEN);
  
  ball = true;
}

void loop() {
  bool down = Touch_getXY();
  left.press(down && left.contains(pixel_x, pixel_y));
  right.press(down && right.contains(pixel_x, pixel_y));
  if(right.justPressed() && x < tft.width() - 80) {
    tft.fillRect(x, y, 2, 10, BLACK);
    x++;
    tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  }
  if(left.justPressed() && x > 0) {
    tft.fillRect(x + block * 2 - 2, y, 2, 10, BLACK);
    x--;
    tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  }
  Serial.println("pressed");
  if(ball) {
    tft.drawCircle(xB, yB, 9, BLACK);
    tft.drawLine(xB - 5, 0, xB + 5, 0, BLACK);
    xB = xB + cX;
    yB = yB + cY;
    Serie2.write(0xFF);
    Serie2.write(xB);
    Serie2.write(yB);
    tft.fillCircle(xB, yB, 8, BLACK);
    tft.drawCircle(xB, yB, 9, GREEN);
    tft.drawLine(xB - 5, 0, xB + 5, 0, Orange);
    if(xB == 240 - 10 || xB == 10)
      cX = -cX;
    if(yB == y - 10/* || yB == 10*/)
      cY = -cY;
    if(yB == 0) {
      ball = false;
      Serie2.write('1');
    }
    else
      Serie2.write('0');
     while(!Serie2.available()){}
  }
  else if(Serie2.available() > 3) {
    if(Serie2.read() == 0xFF) {
      tft.drawCircle(xB, yB, 9, BLACK);
      tft.drawLine(xB - 5, 0, xB + 5, 0, BLACK);
      xB = Serie2.read();
      int yBAux = Serie2.read();
      ball = (Serie2.read() == '1' ? true: false);
      if(yBAux <= 10) {
        cY = 1;
        /*if(yBAux == 10) {
          yB = -10;
          cY = 1;
        }
        else if(yBAux < 10)
          yB = yB + cY;*/
        yB = -yBAux;
        tft.fillCircle(xB, yB, 8, BLACK);
        tft.drawCircle(xB, yB, 9, GREEN);
        tft.drawLine(xB - 5, 0, xB + 5, 0, Orange);
      }
    }
  }
}

bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT); //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH); //TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if(pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}
