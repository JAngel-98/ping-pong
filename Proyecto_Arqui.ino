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
Adafruit_GFX_Button left, right, finalb;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
int16_t block;
uint16_t ID, x, y;       //Coordenadas barra
uint16_t xB, yB;         //Coordenadas pelota
uint16_t yButton;        //Cordenadas botones
int8_t cX, cY;  //Avance pelota
uint8_t Orientation = 0;

int bricks[5][12];

int colors[] = {0x001f, 0xf800, 0x07e0, 0x780f, 0xffe0, 0xfd20};

bool playGame = false;
bool lostGame = false;

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

void setup(void) {
  tft.reset();
  ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(Orientation);
  tft.fillScreen(BLACK);
  
  //Serial.begin(9600);

  block = tft.width() / 6;
  tft.fillScreen(BLACK);
  
  randomSeed(analogRead(13));
  restartBricks();
  
  startGame();
  
  left.initButton(&tft,  20, yButton, 40, 20, WHITE, BLACK, WHITE, "Left", 1);
  right.initButton(&tft, 240 - 20, yButton, 40, 20, WHITE, BLACK, WHITE, "Right", 1);
  left.drawButton(false);
  right.drawButton(false);
}

void loop() {
  if(!playGame && !lostGame) {
    bool down = Touch_getXY();
    left.press(down && left.contains(pixel_x, pixel_y));
    right.press(down && right.contains(pixel_x, pixel_y));
    if(right.justPressed() || left.justPressed()) {
      tft.fillRect(0, 100, 240, 150, BLACK);
      playGame = true;
      
    }
  }
  if(!playGame && lostGame) {
    bool down = Touch_getXY();
    finalb.press(down && finalb.contains(pixel_x, pixel_y));
    if(finalb.justPressed()){
      lostGame = false;
      restartGame();
    }
  }
  while(playGame) {
    bool down = Touch_getXY();
    left.press(down && left.contains(pixel_x, pixel_y));
    right.press(down && right.contains(pixel_x, pixel_y));
    if(right.justPressed() && x < tft.width() - block * 2) {
      tft.fillRect(x, y, 8, 10, BLACK);
      x += 4;
      tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
    }
    if(left.justPressed() && x > 0) {
      tft.fillRect(x + block * 2 - 8, y, 8, 10, BLACK);
      x -= 4;
      tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
    }
    tft.fillCircle(xB, yB, 9, BLACK);
    xB = xB + cX;
    yB = yB + cY;
    tft.fillCircle(xB, yB, 9, WHITE);
    tft.drawCircle(xB, yB, 9, DarkGrey);
    if(xB == 240 - 10 || xB == 10)
      cX = -cX;
    if(yB == 10 || (yB == y - 10 && xB >= x && xB <= x + 80))
      cY = -cY;
    if(yB >= 290) {
      lostGame = true;
      playGame = false;
      //randomSeed(analogRead(13));
      gameOver();
      break;
    }
    if(yB < 111 && yB != 10) {
      if((yB - 10) % 20 == 0) {
        if(cY >= 0 && bricks[(int) ((yB + 10 - 20) / 20)][(int) (xB / 20)] == 1) {
          bricks[(int) ((yB + 10 - 20) / 20)][(int) (xB / 20)] = 0;
          tft.fillRect((int) (xB / 20) * 20, (int) (yB / 20) * 20 - 20, 20, 20, BLACK);
          cY = -cY;
        }
        if(cY < 0 && bricks[(int) ((yB - 10 - 20) / 20)][(int) (xB / 20)] == 1) {
          bricks[(int) ((yB - 10 - 20) / 20)][(int) (xB / 20)] = 0;
          tft.fillRect((int) (xB / 20) * 20, (int) (yB / 20) * 20 - 20, 20, 20, BLACK);
          cY = -cY;
        }
      }
      if((xB - 10) % 20 == 0) {
        //
      }
    }
  }
}

void restartBricks() {
  for(int j = 0; j < 5; j++) {
    for(int i = 0; i < 12; i++) {
      bricks[j][i] = 1;
      tft.fillRect(i * 20, j * 20, 20, 20, colors[random(0, 6)]);
      tft.drawRect(i * 20, j * 20, 20, 20, WHITE);
    }
  }
}

void startGame() {
  cX = 1;
  cY = -1; //Avance pelota
  
  x = 80;
  y = 280; //Barra
  xB = x + 40;
  yB = y - 10; //Pelota
  yButton = 310; //Altura Botones
  
  //Dibuja la barra
  tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  //Rellena el interior de la pelota
  tft.fillCircle(xB, yB, 9, WHITE);
  //Dibuja el contorno de la pelota
  tft.drawCircle(xB, yB, 9, DarkGrey);
  //Pintar letras de inicio
  tft.setTextSize(3);
  tft.setCursor(43, 170);
  tft.print("Press a");
  tft.setCursor(43, 205);
  tft.print("button...");
  tft.setTextSize(1);
}

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

void gameOver() {
  finalb.initButton(&tft, 120, 250, 170, 30, WHITE, BLACK, WHITE, " ", 1);
  tft.setCursor(20, 120);
  tft.setTextSize(5);
  tft.print("GAME");
  tft.setCursor(90, 170);
  tft.print("OVER");
  finalb.drawButton(false);
  tft.setTextSize(2);
  tft.setCursor(43, 243);
  tft.print("Press to play");
}
void restartGame (){
  tft.fillRect(0, 100, 240, 200, BLACK);
  restartBricks();
  startGame(); 
}
