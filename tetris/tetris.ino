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
#define TETROMINO_COLOR 0x07E0  // Green
#define BACKGROUND_COLOR 0x0000 // Black

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

// Tetris grid dimensions
#define GRID_WIDTH 10
#define GRID_HEIGHT 20
#define BLOCK_SIZE 6 // Each block is 6x6 pixels

// Game variables
int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 0=empty, 1=filled
int currentTetromino[4][4];              // Current tetromino
int tetrominoX = 3, tetrominoY = 0;      // Starting position of tetromino
int score = 0;
bool gameOver = false;

// Tetromino shapes (I, O, T, L, J, Z, S)
const int tetrominoShapes[7][4][4] = {
  { {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // I
  { {1, 1}, {1, 1} },                                          // O
  { {0, 1, 0}, {1, 1, 1}, {0, 0, 0} },                         // T
  { {1, 0, 0}, {1, 1, 1}, {0, 0, 0} },                         // L
  { {0, 0, 1}, {1, 1, 1}, {0, 0, 0} },                         // J
  { {1, 1, 0}, {0, 1, 1}, {0, 0, 0} },                         // Z
  { {0, 1, 1}, {1, 1, 0}, {0, 0, 0} }                          // S
};

// Function to initialize the game
void initGame() {
  // Clear the grid
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      grid[y][x] = 0;
    }
  }
  
  score = 0;
  gameOver = false;
  spawnTetromino();
  drawGrid();
  drawScore();
}

// Function to spawn a new tetromino
void spawnTetromino() {
  // Pick a random tetromino shape
  int shapeIndex = random(0, 7);
  
  // Copy the shape to the current tetromino
  memset(currentTetromino, 0, sizeof(currentTetromino));
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      currentTetromino[y][x] = tetrominoShapes[shapeIndex][y][x];
    }
  }
  
  tetrominoX = 3;
  tetrominoY = 0;
}

// Function to draw the grid and tetrominoes
void drawGrid() {
  tft.fillScreen(BLACK);
  
  // Draw the grid blocks
  for (int y = 0; y < GRID_HEIGHT; y++) {
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == 1) {
        tft.fillRect(x * BLOCK_SIZE, y * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, TETROMINO_COLOR);
      }
    }
  }

  // Draw the current tetromino
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentTetromino[y][x] == 1) {
        tft.fillRect((tetrominoX + x) * BLOCK_SIZE, (tetrominoY + y) * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, TETROMINO_COLOR);
      }
    }
  }
}

// Function to move the tetromino down
void moveDown() {
  tetrominoY++;
  
  if (checkCollision()) {
    tetrominoY--;
    lockTetromino();
    spawnTetromino();
    if (checkCollision()) {
      gameOver = true;
    }
  }
  
  drawGrid();
}

// Function to move the tetromino left
void moveLeft() {
  tetrominoX--;
  if (checkCollision()) tetrominoX++;
  drawGrid();
}

// Function to move the tetromino right
void moveRight() {
  tetrominoX++;
  if (checkCollision()) tetrominoX--;
  drawGrid();
}

// Function to rotate the tetromino
void rotateTetromino() {
  int temp[4][4] = {0};
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      temp[x][3 - y] = currentTetromino[y][x];
    }
  }
  
  memcpy(currentTetromino, temp, sizeof(temp));
  
  if (checkCollision()) {
    // Undo rotation if it causes a collision
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 4; x++) {
        temp[3 - x][y] = currentTetromino[y][x];
      }
    }
    memcpy(currentTetromino, temp, sizeof(temp));
  }
  
  drawGrid();
}

// Function to check for collisions
bool checkCollision() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentTetromino[y][x] == 1) {
        int newX = tetrominoX + x;
        int newY = tetrominoY + y;
        
        // Check for out-of-bounds or collision with other blocks
        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT || grid[newY][newX] == 1) {
          return true;
        }
      }
    }
  }
  return false;
}

// Function to lock the tetromino in place
void lockTetromino() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentTetromino[y][x] == 1) {
        grid[tetrominoY + y][tetrominoX + x] = 1;
      }
    }
  }
  
  checkLines();
}

// Function to check and clear full lines
void checkLines() {
  for (int y = 0; y < GRID_HEIGHT; y++) {
    bool fullLine = true;
    
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == 0) {
        fullLine = false;
        break;
      }
    }
    
    if (fullLine) {
      // Clear the line and shift everything down
      for (int newY = y; newY > 0; newY--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          grid[newY][x] = grid[newY - 1][x];
        }
      }
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[0][x] = 0;
      }
      
      // Increase the score
      score++;
      drawScore();
    }
  }
}

// Function to draw the score
void drawScore() {
  tft.fillRect(0, 0, SCREEN_WIDTH, 10, BLACK);  // Clear score area
  tft.setCursor(5, 5);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Score: ");
  tft.print(score);
}

// Function to setup the game environment
void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(BLACK);
  randomSeed(analogRead(0));
  
  initGame();
}

// Main game loop
void loop() {
  if (!gameOver) {
    // Move the tetromino down at regular intervals
    static unsigned long lastUpdateTime = 0;
    if (millis() - lastUpdateTime >= 500) { // Move down every 500ms
      moveDown();
      lastUpdateTime = millis();
    }

    // Basic controls (Assuming using buttons or a keypad)
    // Here you should replace with your actual button reading code
    int move = random(0,3);
    
    if (move == 0) {
      moveLeft();
    }
    else if (move == 1) {
      moveRight();
    }
    else if (move == 2) {
      rotateTetromino();
    }

    /*
    if (digitalRead(LEFT_BUTTON_PIN) == HIGH) {
      moveLeft();
    }
    if (digitalRead(RIGHT_BUTTON_PIN) == HIGH) {
      moveRight();
    }
    if (digitalRead(ROTATE_BUTTON_PIN) == HIGH) {
      rotateTetromino();
    }
    if (digitalRead(DOWN_BUTTON_PIN) == HIGH) {
      moveDown();
    }*/
    //rotateTetromino();
    
    // Draw everything
    drawGrid();
  } else {
    // Handle game over (display message, reset, etc.)
    tft.setCursor(20, 50);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Game Over!");
    tft.setCursor(20, 70);
    tft.print("Score: ");
    tft.print(score);
    delay(2000);  // Pause for a moment before resetting
    initGame();
  }
}
