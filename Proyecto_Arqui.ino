#include <SPI.h>          // f.k. for Arduino-1.5.2
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#define MINPRESSURE 200
#define MAXPRESSURE 1000

const int XP = 8, XM = A2, YP = A3, YM = 9; //ID=0x9341
const int TS_LEFT = 117, TS_RT = 897, TS_TOP = 76, TS_BOT = 886;

MCUFRIEND_kbv tft;        // hard-wired for UNO shields anyway.
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
Adafruit_GFX_Button left, right, finalB;

int pixel_x, pixel_y;     //Touch_getXY() updates global vars
int16_t block;
uint16_t ID, x, y;        //Coordenadas barra
float xB, yB;             //Coordenadas pelota
float cX, cY;             //Avance pelota
uint16_t yButton;         //Cordenadas botones
uint8_t Orientation = 0;
int points;

int bricks[16][12] = {0}; //Ladrillos

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

  block = tft.width() / 6;
  tft.fillScreen(BLACK);
  
  randomSeed(analogRead(13));
  
  restartBricks(); //Inicia juego
  startGame();
  
  finalB.initButton(&tft, 120, 250, 170, 30, WHITE, BLACK, WHITE, " ", 1); //Inicia botones
  left.initButton(&tft,  20, yButton, 40, 20, WHITE, BLACK, WHITE, "Left", 1);
  left.drawButton(false);
  right.initButton(&tft, 240 - 20, yButton, 40, 20, WHITE, BLACK, WHITE, "Right", 1);
  right.drawButton(false);
  
}

void loop() {
  if(!playGame && !lostGame) {
    bool down = Touch_getXY();
    left.press(down && left.contains(pixel_x, pixel_y));
    right.press(down && right.contains(pixel_x, pixel_y));
    if(right.justPressed() || left.justPressed()) {
      tft.fillRect(40, 180, 160, 60, BLACK);
      score();
      playGame = true;
      cX = right.justPressed() ? -1 : 1;
    }
  }
  if(!playGame && lostGame) {
    bool down = Touch_getXY();
    finalB.press(down && finalB.contains(pixel_x, pixel_y));
    if(finalB.justPressed()){
      lostGame = false;
      restartGame();
    }
  }
  while(playGame) {
    bool down = Touch_getXY();
    left.press(down && left.contains(pixel_x, pixel_y));
    right.press(down && right.contains(pixel_x, pixel_y));
    if(right.justPressed() && x < tft.width() - block * 2) {
      tft.fillRect(x, y, 10, 10, BLACK);
      x += 8;
      tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
    }
    if(left.justPressed() && x > 0) {
      tft.fillRect(x + block * 2 - 10, y, 10, 10, BLACK);
      x -= 8;
      tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
    }
    tft.fillCircle(xB, yB, 9, BLACK);
    xB = xB + cX;
    yB = yB + cY;
    tft.fillCircle(xB, yB, 9, WHITE);
    tft.drawCircle(xB, yB, 9, DarkGrey);
    if(xB >= 240 - 10 || xB <= 10)
      cX = -cX;
    if(yB <= 10) {
      cY = -cY;
    }
    else if((int) yB == y - 10 && xB >= x && xB <= x + 80){
      if(xB < x + 15 || xB > x + 65) {
        xB = (int) (xB / 2) * 2;
        cX = xB < x + 40 ? -2 : 2;
        cY = -0.5;
      }
      else {
        cX = xB < x + 20 ? -abs(cX) : xB > x + 60 ? abs(cX) : cX;
        cY = -1;
      }
    }
    if(yB >= 290) {
      lostGame = true;
      playGame = false;
      gameOver();
      break;
    }
    if(yB > 10) {
      if(hitBrick(xB, yB - 10)) {
        cY = -cY;
      }
      if(hitBrick(xB + 10, yB)) {
        cX = -cX;
      }
      if(hitBrick(xB, yB + 10)) {
        cY = -cY;
      }
      if(hitBrick(xB - 10, yB)) {
        cX = -cX;
      }
      if(hitBrick(xB + 9, yB - 9)) {
        cX = -1;
        cY = 1;
      }
      if(hitBrick(xB + 9, yB + 9)) {
        cX = -1;
        cY = -1;
      }
      if(hitBrick(xB - 9, yB + 9)) {
        cX = 1;
        cY = -1;
      }
      if(hitBrick(xB - 9, yB - 9)) {
        cX = 1;
        cY = 1;
      }
    }
    if(points == 60) {
      lostGame = true;
      playGame = false;
      winner();
      break;
    }
  }
}

bool hitBrick(int x, int y) {
  if(bricks[y / 20][x / 20] == 1) {
    bricks[y / 20][x / 20] = 0;
    tft.fillRect((int) (x / 20) * 20, (int) (y / 20) * 20, 20, 20, BLACK);
    points++;
    score();
    return true;
  }
  return false;
}

void score() {
  tft.fillRect(60, 300, 120, 20, BLACK);
  tft.setCursor(60, 305);
  tft.print("Score: ");
  tft.print(points * 50);
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
  cY = -1; //Avance Y pelota
  
  x = 80; //Barra
  y = 280;
  xB = x + 40; //Pelota
  yB = y - 10;
  yButton = 310; //Altura Botones

  points = 0;
  
  tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE); //Dibuja la barra
  tft.fillCircle(xB, yB, 9, WHITE); //Rellena el interior de la pelota
  tft.drawCircle(xB, yB, 9, DarkGrey); //Dibuja el contorno de la pelota
  
  tft.setTextSize(3); //Pintar letras de inicio
  tft.setCursor(40, 180);
  tft.print("Press a");
  tft.setCursor(40, 216);
  tft.print("button...");
  tft.setTextSize(1);
  tft.fillRect(60, 300, 120, 20, BLACK);
}

void gameOver() {
  tft.setCursor(20, 120);
  tft.setTextSize(5);
  tft.print("GAME");
  tft.setCursor(90, 170);
  tft.print("OVER");
  
  finalB.drawButton(false);
  tft.setTextSize(2);
  tft.setCursor(43, 243);
  tft.print("Press to play");
}

void winner() {
  tft.setCursor(25, 130);
  tft.setTextSize(4);
  tft.print("You Win!");
  
  tft.setTextSize(2);
  tft.setCursor(20, 200);
  tft.print("Final Score: ");
  tft.print(points * 50);
  
  finalB.drawButton(false);
  tft.setTextSize(2);
  tft.setCursor(43, 243);
  tft.print("Press to play");
}

void restartGame (){
  tft.fillRect(0, 100, 240, 200, BLACK);
  restartBricks();
  startGame(); 
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
