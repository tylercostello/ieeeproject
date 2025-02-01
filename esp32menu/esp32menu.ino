// Menu file: main_menu.ino
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define SCLK_PIN 18
#define MOSI_PIN 23
#define DC_PIN 25
#define CS_PIN 26
#define RST_PIN 27

const int buttonLeftPin = 12;
const int buttonRightPin = 14;
const int buttonUpPin = 32;
const int buttonDownPin = 33;
const int buttonMenuPin = 4;

#define BLACK 0x0000
#define WHITE 0xFFFF

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

// Menu variables
int menuIndex = 0;
const char *menuItems[] = {"Snake", "Pong", "Tetris"};
const int menuItemCount = 3;

int lastMenuButtonState = HIGH;

// Snake game header
extern void runSnakeGame();

// Pong game header
extern void runPongGame();

extern void runTetrisGame();

void drawMenu()
{
    tft.fillScreen(BLACK);
    for (int i = 0; i < menuItemCount; i++)
    {
        tft.setCursor(20, 30 + i * 20);
        tft.setTextColor(i == menuIndex ? WHITE : 0x7BEF); // Highlight selected item
        tft.setTextSize(2);
        tft.print(menuItems[i]);
    }
}

void setup()
{
    pinMode(buttonUpPin, INPUT_PULLUP);
    pinMode(buttonDownPin, INPUT_PULLUP);
    pinMode(buttonLeftPin, INPUT_PULLUP);
    pinMode(buttonRightPin, INPUT_PULLUP);
    pinMode(buttonMenuPin, INPUT_PULLUP);
    EEPROM.begin(512); // Initialize EEPROM with 512 bytes
    Serial.begin(9600);
    tft.begin();
    tft.fillScreen(BLACK);
    drawMenu();
}

void loop()
{
    if (digitalRead(buttonUpPin) == LOW)
    {
        menuIndex = (menuIndex - 1 + menuItemCount) % menuItemCount;
        drawMenu();
        delay(200);
    }
    else if (digitalRead(buttonDownPin) == LOW)
    {
        menuIndex = (menuIndex + 1) % menuItemCount;
        drawMenu();
        delay(200);
    }
    else if (digitalRead(buttonRightPin) == LOW && lastMenuButtonState == HIGH)
    {
        EEPROM.commit();
        tft.fillScreen(BLACK);
        delay(200);
        if (menuIndex == 0)
        {
            runSnakeGame();
        }
        else if (menuIndex == 1)
        {
            runPongGame();
        }
        else if (menuIndex == 2)
        {
            runTetrisGame();
        }

        drawMenu();
    }
    lastMenuButtonState = digitalRead(buttonRightPin);
}
