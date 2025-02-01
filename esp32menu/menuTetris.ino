#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

extern Adafruit_SSD1351 tft;
extern const int buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonMenuPin;
extern void checkPauseMenu(void (*resetGame)(), void (*exitGame)());
const int buttonRotatePin = buttonUpPin;

// High score variables
const int tetrisHighScoreAddress = 100; // Different address from snake game
int tetrisHighScore = 0;

// Tetris grid dimensions
#define GRID_WIDTH 10
#define GRID_HEIGHT 20
#define BLOCK_SIZE 6

#define BLACK          0x0000
#define WHITE          0xFFFF
#define GRAY           0x4208
#define BACKGROUND_COLOR BLACK

// Game variables
int grid[GRID_HEIGHT][GRID_WIDTH] = {0};
int currentTetromino[4][4];
int tetrominoX = 3, tetrominoY = 0;
int prevTetrominoX = 3;
int prevTetrominoY = 0;
int prevTetromino[4][4] = {0};
int tetrisScore = 0;
bool gameOverTetris = false;
uint16_t currentColor;
const int tetrisScores[4] = {40, 100, 300, 1200};
int tetrisDelay = 501;
int totalLinesCleared = 0;
bool exitGameBoolTetris = false;

// Tetromino definitions
const int tetrominoShapes[7][4][4] = {
    {{1,1,1,1}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{1,1,0,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{1,0,0,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,1,0}, {1,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{1,1,0,0}, {0,1,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,1,1,0}, {1,1,0,0}, {0,0,0,0}, {0,0,0,0}}
};

const uint16_t tetrominoColors[7] = {
    0x07FF, 0xFFE0, 0xF81F, 0xFD20, 0x001F, 0xF800, 0x07E0
};

int nextTetromino[4][4] = {0};
uint16_t nextColor;
bool lossScreenTetris = false;

// Button states
int buttonLeftState = 0;
int buttonRightState = 0;
int buttonRotateState = 0;
int buttonDownState = 0;
int lastButtonLeftState = HIGH;
int lastButtonRightState = HIGH;
int lastButtonRotateState = HIGH;
int lastButtonDownState = HIGH;

void pauseDelayTetris(int ms) {
    unsigned long start = millis();
    while (millis() - start < ms) {
        if (digitalRead(buttonMenuPin) == LOW) {
            checkPauseMenu(initTetrisGame, exitGameTetris);
            if (lossScreenTetris) {
                tft.fillScreen(BLACK);
                tft.setCursor(5, 30);
                tft.setTextColor(WHITE);
                tft.setTextSize(2);
                tft.print("Game Over!");

                tft.setCursor(0, 50);
                tft.setTextSize(1);
                tft.print("Score: ");
                tft.print(tetrisScore);

                tft.setCursor(0, 70);
                tft.print("High Score: ");
                tft.print(tetrisHighScore);
            } else {
                drawNextTetromino();
            }
        }
    }
}

void exitGameTetris() {
    gameOverTetris = false;
    exitGameBoolTetris = true;
}

void initTetrisGame() {
    exitGameBoolTetris = false;
    lossScreenTetris = false;
    
    // Initialize EEPROM and load high score
    EEPROM.begin(512); // Adjust this based on available EEPROM size

    EEPROM.get(tetrisHighScoreAddress, tetrisHighScore);
    EEPROM.end();
    
    if (tetrisHighScore < 0) tetrisHighScore = 0;

    randomSeed(micros());
    for (int y = 0; y < GRID_HEIGHT; y++) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            grid[y][x] = 0;
        }
    }

    tetrisScore = 0;
    totalLinesCleared = 0;
    gameOverTetris = false;
    tft.fillScreen(BLACK);

    int shapeIndex = random(0, 7);
    nextColor = tetrominoColors[shapeIndex];
    memset(nextTetromino, 0, sizeof(nextTetromino));
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            nextTetromino[y][x] = tetrominoShapes[shapeIndex][y][x];
        }
    }

    spawnTetromino();
    drawGrid();
    drawTetrisScore();
    drawNextTetromino();
}

void spawnTetromino() {
    memcpy(currentTetromino, nextTetromino, sizeof(currentTetromino));
    currentColor = nextColor;
    tetrominoX = 3;
    tetrominoY = 0;

    int shapeIndex = random(0, 7);
    nextColor = tetrominoColors[shapeIndex];
    memset(nextTetromino, 0, sizeof(nextTetromino));
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            nextTetromino[y][x] = tetrominoShapes[shapeIndex][y][x];
        }
    }
    drawNextTetromino();
}

void drawNextTetromino() {
    tft.fillRect(80, 40, 40, 40, BLACK);
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (nextTetromino[y][x] == 1) {
                int screenX = 80 + x * BLOCK_SIZE;
                int screenY = 50 + y * BLOCK_SIZE;
                tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, nextColor);
            }
        }
    }
    tft.setCursor(80, 35);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.print("Next:");
}

void drawGrid() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (prevTetromino[y][x] == 1) {
                int screenX = (prevTetrominoX + x) * BLOCK_SIZE;
                int screenY = (prevTetrominoY + y) * BLOCK_SIZE;
                if (prevTetrominoY + y < GRID_HEIGHT && prevTetrominoX + x < GRID_WIDTH &&
                    grid[prevTetrominoY + y][prevTetrominoX + x] == 0) {
                    tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, BLACK);
                }
            }
        }
    }

    for (int x = 0; x <= GRID_WIDTH; x++) {
        tft.drawFastVLine(x * BLOCK_SIZE, 0, GRID_HEIGHT * BLOCK_SIZE, GRAY);
    }
    for (int y = 0; y <= GRID_HEIGHT; y++) {
        tft.drawFastHLine(0, y * BLOCK_SIZE, GRID_WIDTH * BLOCK_SIZE, GRAY);
    }

    tft.drawFastVLine(GRID_WIDTH * BLOCK_SIZE, 0, GRID_HEIGHT * BLOCK_SIZE, GRAY);

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentTetromino[y][x] == 1) {
                int screenX = (tetrominoX + x) * BLOCK_SIZE;
                int screenY = (tetrominoY + y) * BLOCK_SIZE;
                tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, currentColor);
            }
        }
    }

    prevTetrominoX = tetrominoX;
    prevTetrominoY = tetrominoY;
    memcpy(prevTetromino, currentTetromino, sizeof(prevTetromino));
}

void moveDown() {
    tetrominoY++;
    if (checkCollision()) {
        tetrominoY--;
        lockTetromino();
        spawnTetromino();
        if (checkCollision()) gameOverTetris = true;
    }
    drawGrid();
}

void moveLeft() {
    tetrominoX--;
    if (checkCollision()) tetrominoX++;
    drawGrid();
}

void moveRight() {
    tetrominoX++;
    if (checkCollision()) tetrominoX--;
    drawGrid();
}

void rotateTetromino() {
    int temp[4][4] = {0};
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            temp[x][3 - y] = currentTetromino[y][x];
        }
    }

    int tempX = tetrominoX;
    int tempY = tetrominoY;
    int tempShape[4][4];
    memcpy(tempShape, currentTetromino, sizeof(tempShape));
    memcpy(currentTetromino, temp, sizeof(temp));

    if (checkCollision()) {
        tetrominoX = tempX;
        tetrominoY = tempY;
        memcpy(currentTetromino, tempShape, sizeof(currentTetromino));
    } else {
        drawGrid();
    }
}

bool checkCollision() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentTetromino[y][x] == 1) {
                int newX = tetrominoX + x;
                int newY = tetrominoY + y;
                if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
                    (newY >= 0 && grid[newY][newX] == 1)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void lockTetromino() {
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            if (currentTetromino[y][x] == 1) {
                int gridY = tetrominoY + y;
                int gridX = tetrominoX + x;
                if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
                    grid[gridY][gridX] = 1;
                    tft.fillRect(gridX * BLOCK_SIZE, gridY * BLOCK_SIZE, 
                               BLOCK_SIZE, BLOCK_SIZE, currentColor);
                }
            }
        }
    }
    checkLines();
}

void checkLines() {
    int linesCleared = 0;
    for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
        bool fullLine = true;
        for (int x = 0; x < GRID_WIDTH; x++) {
            if (grid[y][x] == 0) {
                fullLine = false;
                break;
            }
        }

        if (fullLine) {
            tft.fillRect(0, y * BLOCK_SIZE, GRID_WIDTH * BLOCK_SIZE, BLOCK_SIZE, BLACK);
            for (int newY = y; newY > 0; newY--) {
                for (int x = 0; x < GRID_WIDTH; x++) {
                    grid[newY][x] = grid[newY - 1][x];
                    if (grid[newY][x] == 1) {
                        tft.fillRect(x * BLOCK_SIZE, newY * BLOCK_SIZE,
                                   BLOCK_SIZE, BLOCK_SIZE, currentColor);
                    } else {
                        tft.fillRect(x * BLOCK_SIZE, newY * BLOCK_SIZE,
                                   BLOCK_SIZE, BLOCK_SIZE, BLACK);
                    }
                }
            }
            for (int x = 0; x < GRID_WIDTH; x++) grid[0][x] = 0;
            linesCleared++;
            totalLinesCleared++;
            y++;
        }
    }

    int level = totalLinesCleared / 10;
    tetrisScore += tetrisScores[linesCleared - 1] * (level + 1);
    tetrisDelay = max(50, 500 - level * 50);
    drawTetrisScore();
}

void drawTetrisScore() {
    tft.fillRect(65, 5, SCREEN_WIDTH, 30, BLACK);
    tft.setCursor(80, 5);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.print("Score: ");
    tft.setCursor(80, 15);
    tft.print(tetrisScore);
}

void runTetrisGame() {
    initTetrisGame();
    while (!exitGameBoolTetris) {
        if (!gameOverTetris) {
            static unsigned long lastUpdateTime = 0;
            unsigned long currentTime = millis();
            if (currentTime - lastUpdateTime >= tetrisDelay) {
                moveDown();
                drawTetrisScore();
                lastUpdateTime = currentTime;
            }

            buttonLeftState = digitalRead(buttonLeftPin);
            buttonRightState = digitalRead(buttonRightPin);
            buttonRotateState = digitalRead(buttonRotatePin);
            buttonDownState = digitalRead(buttonDownPin);

            if (buttonLeftState == LOW && lastButtonLeftState == HIGH) {
                moveLeft();
                drawTetrisScore();
            }
            if (buttonRightState == LOW && lastButtonRightState == HIGH) {
                moveRight();
                drawTetrisScore();
            }
            if (buttonRotateState == LOW && lastButtonRotateState == HIGH) {
                rotateTetromino();
                drawTetrisScore();
            }
            if (buttonDownState == LOW && lastButtonDownState == HIGH) {
                moveDown();
                drawTetrisScore();
            }

            lastButtonLeftState = buttonLeftState;
            lastButtonRightState = buttonRightState;
            lastButtonRotateState = buttonRotateState;
            lastButtonDownState = buttonDownState;
            pauseDelayTetris(50);
        } else {
            lossScreenTetris = true;
            tft.fillScreen(BLACK);
            tft.setCursor(5, 30);
            tft.setTextColor(WHITE);
            tft.setTextSize(2);
            tft.print("Game Over!");

            tft.setCursor(0, 50);
            tft.setTextSize(1);
            tft.print("Score: ");
            tft.print(tetrisScore);

            tft.setCursor(0, 70);
            tft.print("High Score: ");
            tft.print(tetrisHighScore);

            if (tetrisScore > tetrisHighScore) {
                tetrisHighScore = tetrisScore;
                EEPROM.begin(tetrisHighScoreAddress + sizeof(int));
                EEPROM.put(tetrisHighScoreAddress, tetrisHighScore);
                EEPROM.commit();
                EEPROM.end();
                
                tft.setCursor(0, 110);
                tft.print("New High Score!");
            }

            while (lossScreenTetris) {
                if (digitalRead(buttonLeftPin) == LOW || digitalRead(buttonRightPin) == LOW ||
                    digitalRead(buttonRotatePin) == LOW || digitalRead(buttonDownPin) == LOW) {
                    pauseDelayTetris(200);
                    initTetrisGame();
                    break;
                }
                pauseDelayTetris(10);
                if (exitGameBoolTetris) break;
            }
            lossScreenTetris = false;
        }
    }
}