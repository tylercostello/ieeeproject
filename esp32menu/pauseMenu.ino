#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

extern Adafruit_SSD1351 tft; // Shared display object
extern const int buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonMenuPin;



#define BLACK 0x0000
#define WHITE 0xFFFF
#define GRAY 0x4208

// Pause menu variables
int pauseMenuIndex = 0;
const char *pauseMenuItems[] = {"Resume", "Reset", "Exit"};
const int pauseMenuItemCount = 3;
bool isPaused = false;

void drawPauseMenu() {
    tft.fillScreen(BLACK);
    for (int i = 0; i < pauseMenuItemCount; i++) {
        tft.setCursor(20, 30 + i * 20);
        tft.setTextColor(i == pauseMenuIndex ? WHITE : GRAY); // Highlight selected item
        tft.setTextSize(2);
        tft.print(pauseMenuItems[i]);
    }
}

void handlePauseMenu(void (*resetGame)(), void (*exitGame)()) {
    if (digitalRead(buttonUpPin) == LOW) {
        pauseMenuIndex = (pauseMenuIndex - 1 + pauseMenuItemCount) % pauseMenuItemCount;
        drawPauseMenu();
        delay(200);
    } else if (digitalRead(buttonDownPin) == LOW) {
        pauseMenuIndex = (pauseMenuIndex + 1) % pauseMenuItemCount;
        drawPauseMenu();
        delay(200);
    } else if (digitalRead(buttonRightPin) == LOW) {
        if (pauseMenuIndex == 0) { // Resume Game
            tft.fillScreen(BLACK);
            isPaused = false;
        } else if (pauseMenuIndex == 1) { // Reset Game
            isPaused = false;
            resetGame();
        } else if (pauseMenuIndex == 2) { // Exit Game
            isPaused = false;
            exitGame();
        }
        delay(200);
    }
}

void checkPauseMenu(void (*resetGame)(), void (*exitGame)()) {
    isPaused = true;
    drawPauseMenu();
    while (isPaused) {
        handlePauseMenu(resetGame, exitGame);
        delay(10); // Small delay to prevent excessive CPU usage
    }
}
