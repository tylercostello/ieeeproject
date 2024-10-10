#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define SCLK_PIN 13
#define MOSI_PIN 11
#define DC_PIN 4
#define CS_PIN 5
#define RST_PIN 6

#define BLACK 0x0000
#define WHITE 0xFFFF
#define BLOCK_COLOR 0x07E0  // Green
#define EMPTY_COLOR BLACK

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

// Define the size of each block
const int blockSize = 8;

// Define the grid dimensions
const int gridWidth = SCREEN_WIDTH / blockSize;
const int gridHeight = SCREEN_HEIGHT / blockSize;

// Game variables
int grid[16][16] = {0};  // Stores the game grid (0: empty, 1: filled)
int score = 0;           // Stores the player's score

// Tetromino shapes (simplified for a small screen)
int tetrominoes[7][4][4] = {
  { // O-shape
    {1, 1, 0, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // I-shape
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // T-shape
    {0, 1, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // L-shape
    {1, 0, 0, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // J-shape
    {0, 0, 1, 0},
    {1, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // S-shape
    {0, 1, 1, 0},
    {1, 1, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  },
  { // Z-shape
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
  }
};

int currentTetromino = 0;
int tetrominoX = gridWidth / 2 - 2;
int tetrominoY = 0;
int rotation = 0;

void drawBlock(int x, int y, uint16_t color) {
  if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight) {
    tft.fillRect(x * blockSize, y * blockSize, blockSize, blockSize, color);
  }
}

void drawGrid() {
  for (int y = 0; y < gridHeight; y++) {
    for (int x = 0; x < gridWidth; x++) {
      if (grid[y][x] == 1) {
        drawBlock(x, y, BLOCK_COLOR);
      } else {
        drawBlock(x, y, EMPTY_COLOR);
      }
    }
  }
}

void drawTetromino(int x, int y, int shape[4][4], uint16_t color) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (shape[i][j] == 1) {
        drawBlock(x + j, y + i, color);
      }
    }
  }
}

bool canMove(int newX, int newY, int newRotation) {
  int newShape[4][4];
  // Get the rotated shape
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      newShape[i][j] = tetrominoes[currentTetromino][(i + newRotation) % 4][j];
    }
  }
  // Check for collisions
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (newShape[i][j] == 1) {
        int newXPos = newX + j;
        int newYPos = newY + i;
        if (newXPos < 0 || newXPos >= gridWidth || newYPos >= gridHeight || grid[newYPos][newXPos] == 1) {
          return false;
        }
      }
    }
  }
  return true;
}

void placeTetromino() {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (tetrominoes[currentTetromino][i][j] == 1) {
        grid[tetrominoY + i][tetrominoX + j] = 1;
      }
    }
  }
}

void clearLine(int line) {
  for (int y = line; y > 0; y--) {
    for (int x = 0; x < gridWidth; x++) {
      grid[y][x] = grid[y - 1][x];
    }
  }
  for (int x = 0; x < gridWidth; x++) {
    grid[0][x] = 0;
  }
}

void checkLines() {
  for (int y = 0; y < gridHeight; y++) {
    bool fullLine = true;
    for (int x = 0; x < gridWidth; x++) {
      if (grid[y][x] == 0) {
        fullLine = false;
        break;
      }
    }
    if (fullLine) {
      clearLine(y);
      score += 100;  // Add points for clearing a line
    }
  }
}

void drawScore() {
  tft.setCursor(0, 0);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.fillRect(0, 0, SCREEN_WIDTH, 10, BLACK); // Erase previous score
  tft.print("Score: ");
  tft.print(score);
}

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(BLACK);
  randomSeed(analogRead(0));

  currentTetromino = random(0, 7);
  drawScore();
}

void loop() {
  // Erase current tetromino by drawing it in black
  drawTetromino(tetrominoX, tetrominoY, tetrominoes[currentTetromino], BLACK);

  // Move the tetromino down if possible
  if (canMove(tetrominoX, tetrominoY + 1, rotation)) {
    tetrominoY++;
  } else {
    // Place the tetromino on the grid
    placeTetromino();
    checkLines();  // Check for full lines and update score
    drawGrid();    // Update the grid display
    drawScore();   // Update the score display

    // Generate a new tetromino
    currentTetromino = random(0, 7);
    tetrominoX = gridWidth / 2 - 2;
    tetrominoY = 0;
    rotation = 0;

    // Check if the new tetromino can be placed, if not, game over
    if (!canMove(tetrominoX, tetrominoY, rotation)) {
      tft.fillScreen(BLACK);
      tft.setCursor(20, SCREEN_HEIGHT / 2);
      tft.setTextColor(WHITE);
      tft.setTextSize(2);
      tft.print("Game Over");
      while (true);  // Halt the game
    }
  }

  // Draw the tetromino in its new position
  drawTetromino(tetrominoX, tetrominoY, tetrominoes[currentTetromino], BLOCK_COLOR);

  delay(500);  // Adjust the speed of the game
}
