#include <SPI.h>          // f.k. for Arduino-1.5.2
#include "Adafruit_GFX.h"// Hardware-specific library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.

int16_t block;
uint16_t ID, x, y, xB, yB;
int9_t cX = 1, cY = -1;
uint8_t Orientation = 0;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

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

  //tft.fillRect(0, 0, block, block, RED);
  //tft.fillRect(block, 0, block, block, YELLOW);
  //tft.fillRect(block * 2, 0, block, block, GREEN);
  //tft.fillRect(block * 3, 0, block, block, CYAN);
  //tft.fillRect(block * 4, 0, block, block, BLUE);
  //tft.fillRect(block * 5, 0, block, block, MAGENTA);

  //tft.drawRect(0, 0, block, block, WHITE);
  //tft.drawRect(block, 0, block, block, WHITE);
  //tft.drawRect(block * 2, 0, block, block, WHITE);
  //tft.drawRect(block * 3, 0, block, block, WHITE);
  //tft.drawRect(block * 4, 0, block, block, WHITE);
  //tft.drawRect(block * 5, 0, block, block, WHITE);
  
  x = block * 2;
  y = block * 7.5;
  xB = x + 40;
  yB = y - 10;
  
  tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  tft.fillCircle(xB, yB, 8, BLUE);
  tft.drawCircle(xB, yB, 9, YELLOW);
}

void loop() {
  xB = xB + cX;
  yB = yB + cY;
  if(digitalRead(13) == HIGH && x < tft.width() - block * 2) {
    tft.fillRoundRect(x, y, 1, 10, 1, BLACK);
    x++;
    tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  }
  if(digitalRead(12) == HIGH && x > 0) {
    tft.fillRoundRect(x + block * 2 - 1, y, 1, 10, 1, BLACK);
    x--;
    tft.fillRoundRect(x, y, block * 2, 10, 1, WHITE);
  }
  tft.fillCircle(xB, yB, 8, BLUE);
  tft.drawCircle(xB, yB, 9, YELLOW);
  if(xB == 240 - 10 || xB == 10) {
    xB = -xB;
  }
  if(yB == 360 - 20 || yB == 10 {
    yB = -yB;
  }
  delay(5);
}

