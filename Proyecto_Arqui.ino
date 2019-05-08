#include <SPI.h>          // f.k. for Arduino-1.5.2
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#define MINPRESSURE 200
#define MAXPRESSURE 1000

const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_LEFT = 117, TS_RT = 897, TS_TOP = 76, TS_BOT = 886;

MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button left, right;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
int16_t block;
uint16_t ID, x, y, xB, yB, yButton;
int8_t cX = 1, cY = -1;
uint8_t Orientation = 0;

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   	0x0000
#define	BLUE    	0x001F
#define	RED     	0xF800
#define	GREEN   	0x07E0
#define CYAN    	0x07FF
#define MAGENTA 	0xF81F
#define YELLOW  	0xFFE0
#define WHITE   	0xFFFF
#define Navy        0x000F
#define DarkGreen   0x03E0
#define DarkCyan    0x03EF
#define Maroon      0x7800
#define Purple      0x780F
#define Olive       0x7BE0
#define LightGrey   0xC618
#define DarkGrey    0x7BEF
#define Orange      0xFD20
#define GreenYellow 0xAFE5 
#define Pink        0xF81F

bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);   //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if(pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}

void setup(void) {
  pinMode(12, INPUT);
  pinMode(13, INPUT);
  
  tft.reset();
  ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(Orientation);
  tft.fillScreen(BLACK);
  
  //Serial.begin(9600);

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
}

void loop() {
  bool down = Touch_getXY();
  left.press(down && left.contains(pixel_x, pixel_y));
  right.press(down && right.contains(pixel_x, pixel_y));
  if((digitalRead(13) == HIGH || right.justPressed()) && x < tft.width() - block * 2) {
    tft.fillRect(x, y, 2, 10, BLACK);
    x++;
    tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  }
  if((digitalRead(12) == HIGH || left.justPressed()) && x > 0) {
    tft.fillRect(x + block * 2 - 2, y, 2, 10, BLACK);
    x--;
    tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  }
  tft.drawCircle(xB, yB, 9, BLACK);
  xB = xB + cX;
  yB = yB + cY;
  //Serial.print("xB: ");
  //Serial.print(xB);
  //Serial.print("\tyB: ");
  //Serial.println(yB);
  tft.fillCircle(xB, yB, 8, BLACK);
  tft.drawCircle(xB, yB, 9, GREEN);
  if(xB == 240 - 10 || xB == 10) {
    cX = -cX;
  }
  if(yB == y - 10 || yB == 10) {
    cY = -cY;
  }
}
