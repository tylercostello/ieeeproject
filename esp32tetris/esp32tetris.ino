#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define SCLK_PIN 18
#define MOSI_PIN 23
#define DC_PIN 25
#define CS_PIN 26
#define RST_PIN 27

#define BLACK 0x0000
#define WHITE 0xFFFF
#define GRAY 0x4208
#define BACKGROUND_COLOR 0x0000 // Black

const int buttonLeftPin = 12;
const int buttonRightPin = 14;
const int buttonRotatePin = 32;
const int buttonDownPin = 33;

// Variables to store button states
int buttonLeftState = 0;
int buttonRightState = 0;
int buttonRotateState = 0;
int buttonDownState = 0;

int lastButtonLeftState = HIGH;
int lastButtonRightState = HIGH;
int lastButtonRotateState = HIGH;
int lastButtonDownState = HIGH;

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

// Tetris grid dimensions
#define GRID_WIDTH 10
#define GRID_HEIGHT 20
#define BLOCK_SIZE 6 // Each block is 6x6 pixels

// Game variables
int grid[GRID_HEIGHT][GRID_WIDTH] = {0}; // 0=empty, 1=filled
int currentTetromino[4][4];              // Current tetromino
int tetrominoX = 3, tetrominoY = 0;      // Starting position of tetromino
int prevTetrominoX = 3;                  // Previous tetromino position
int prevTetrominoY = 0;
int prevTetromino[4][4] = {0};           // Previous tetromino shape
int score = 0;
bool gameOver = false;
uint16_t currentColor;                   // Color of the current tetromino

// Tetromino shapes (I, O, T, L, J, Z, S)
const int tetrominoShapes[7][4][4] = {
  { {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // I
  { {1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // O
  { {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // T
  { {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // L
  { {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // J
  { {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }, // Z
  { {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0} }  // S
};

// Colors for each tetromino shape
const uint16_t tetrominoColors[7] = {
  0x07FF, // Cyan for I
  0xFFE0, // Yellow for O
  0xF81F, // Magenta for T
  0xFD20, // Orange for L
  0x001F, // Blue for J
  0xF800, // Red for Z
  0x07E0  // Green for S
};

// Function prototypes
void initGame();
void spawnTetromino();
void drawGrid();
void moveDown();
void moveLeft();
void moveRight();
void rotateTetromino();
bool checkCollision();
void lockTetromino();
void checkLines();
void drawScore();

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
  
  // Clear the screen once at the start
  tft.fillScreen(BLACK);
  
  spawnTetromino();
  // Initialize previous position
  prevTetrominoX = tetrominoX;
  prevTetrominoY = tetrominoY;
  memcpy(prevTetromino, currentTetromino, sizeof(prevTetromino));
  
  drawGrid();
  drawScore();
}

// Function to spawn a new tetromino
void spawnTetromino() {
  // Pick a random tetromino shape
  int shapeIndex = random(0, 7);
  
  // Set the color for the current tetromino
  currentColor = tetrominoColors[shapeIndex];
  
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
// Function to draw the grid, border, and tetrominoes
void drawGrid() {
  // Clear previous tetromino position
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

  // Draw grid pattern
  for (int x = 0; x <= GRID_WIDTH; x++) {
    int lineX = x * BLOCK_SIZE;
    tft.drawFastVLine(lineX, 0, GRID_HEIGHT * BLOCK_SIZE, GRAY);  // Vertical grid lines
  }
  for (int y = 0; y <= GRID_HEIGHT; y++) {
    int lineY = y * BLOCK_SIZE;
    tft.drawFastHLine(0, lineY, GRID_WIDTH * BLOCK_SIZE, GRAY);   // Horizontal grid lines
  }

  // Right border
  tft.drawFastVLine(GRID_WIDTH * BLOCK_SIZE, 0, GRID_HEIGHT * BLOCK_SIZE, GRAY);

  // Draw current tetromino
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentTetromino[y][x] == 1) {
        int screenX = (tetrominoX + x) * BLOCK_SIZE;
        int screenY = (tetrominoY + y) * BLOCK_SIZE;
        tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, currentColor);
      }
    }
  }

  // Store current position and shape for the next update
  prevTetrominoX = tetrominoX;
  prevTetrominoY = tetrominoY;
  memcpy(prevTetromino, currentTetromino, sizeof(prevTetromino));
}


// Remaining functions (moveDown, moveLeft, moveRight, etc.) remain the same.
// You may now run the code with the added functionality of tetrominoes having different colors.
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

  // Perform rotation
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      temp[x][3 - y] = currentTetromino[y][x];
    }
  }

  // Save current state in case we need to revert
  int tempX = tetrominoX;
  int tempY = tetrominoY;
  int tempShape[4][4];
  memcpy(tempShape, currentTetromino, sizeof(tempShape));

  // Apply rotation
  memcpy(currentTetromino, temp, sizeof(temp));

  // Check if rotation is valid
  if (checkCollision()) {
    // Revert if invalid
    tetrominoX = tempX;
    tetrominoY = tempY;
    memcpy(currentTetromino, tempShape, sizeof(currentTetromino));
  } else {
    drawGrid();
  }
}

// Function to check for collisions
bool checkCollision() {
  for (int y = 0; y < 4; y++) {
    for (int x = 0; x < 4; x++) {
      if (currentTetromino[y][x] == 1) {
        int newX = tetrominoX + x;
        int newY = tetrominoY + y;

        // Check for out-of-bounds or collision with other blocks
        if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT || 
            (newY >= 0 && grid[newY][newX] == 1)) {
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
        int gridY = tetrominoY + y;
        int gridX = tetrominoX + x;
        if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH) {
          grid[gridY][gridX] = 1;
          // Draw the locked block using the tetromino's color
          tft.fillRect(gridX * BLOCK_SIZE, 
                      gridY * BLOCK_SIZE, 
                      BLOCK_SIZE, 
                      BLOCK_SIZE, 
                      currentColor);
        }
      }
    }
  }

  checkLines();
}

// Function to check and clear full lines
void checkLines() {
  for (int y = GRID_HEIGHT - 1; y >= 0; y--) {
    bool fullLine = true;

    // Check if line is full
    for (int x = 0; x < GRID_WIDTH; x++) {
      if (grid[y][x] == 0) {
        fullLine = false;
        break;
      }
    }

    if (fullLine) {
      // Clear the line visually
      tft.fillRect(0, y * BLOCK_SIZE, GRID_WIDTH * BLOCK_SIZE, BLOCK_SIZE, BLACK);

      // Shift everything down
      for (int newY = y; newY > 0; newY--) {
        for (int x = 0; x < GRID_WIDTH; x++) {
          grid[newY][x] = grid[newY - 1][x];
          // Update the display
          if (grid[newY][x] == 1) {
            tft.fillRect(x * BLOCK_SIZE, newY * BLOCK_SIZE, 
                        BLOCK_SIZE, BLOCK_SIZE, currentColor);
          } else {
            tft.fillRect(x * BLOCK_SIZE, newY * BLOCK_SIZE, 
                        BLOCK_SIZE, BLOCK_SIZE, BLACK);
          }
        }
      }

      // Clear the top line
      for (int x = 0; x < GRID_WIDTH; x++) {
        grid[0][x] = 0;
      }

      score++;
      drawScore();

      // Check the same line again as lines might have shifted down
      y++;
    }
  }
}

// Function to draw the score
void drawScore() {
  tft.fillRect(70, 5, SCREEN_WIDTH, 10, BLACK);  // Clear score area
  tft.setCursor(75, 5);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Score: ");
  tft.print(score);
}

// Main setup function
void setup() {
  pinMode(buttonLeftPin, INPUT_PULLUP);
  pinMode(buttonRightPin, INPUT_PULLUP);
  pinMode(buttonRotatePin, INPUT_PULLUP);
  pinMode(buttonDownPin, INPUT_PULLUP);
  Serial.begin(9600);
  tft.begin();
  randomSeed(analogRead(0));
  initGame();
}

// Main game loop
void loop() {
  if (!gameOver) {
    // Move the tetromino down at regular intervals
    static unsigned long lastUpdateTime = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastUpdateTime >= 500) { // Move down every 500ms
      moveDown();
      drawScore();
      lastUpdateTime = currentTime;
    }

    // Check for button inputs
    // Replace these with your actual button pins and logic
  buttonLeftState = digitalRead(buttonLeftPin);
  buttonRightState = digitalRead(buttonRightPin);
  buttonRotateState = digitalRead(buttonRotatePin);
  buttonDownState = digitalRead(buttonDownPin);

  if (buttonLeftState == LOW && lastButtonLeftState == HIGH) {
    Serial.println("Left button pressed");
    moveLeft();
    drawScore();
  }

  // Check if the Right button was pressed (HIGH to LOW transition)
  if (buttonRightState == LOW && lastButtonRightState == HIGH) {
    Serial.println("Right button pressed");
    moveRight();
    drawScore();
  }

  // Check if the Rotate button was pressed (HIGH to LOW transition)
  if (buttonRotateState == LOW && lastButtonRotateState == HIGH) {
    Serial.println("Rotate button pressed");
    rotateTetromino();
    drawScore();
  }

  // Check if the Down button was pressed (HIGH to LOW transition)
  if (buttonDownState == LOW && lastButtonDownState == HIGH) {
    Serial.println("Down button pressed");
    moveDown(); // Assuming you have a function for moving down
    drawScore();
  }

  // Update the previous button states for the next loop
  lastButtonLeftState = buttonLeftState;
  lastButtonRightState = buttonRightState;
  lastButtonRotateState = buttonRotateState;
  lastButtonDownState = buttonDownState;
  delay(50);
    /*

    int move = random(0,100000);

    if (move == 0) {
      moveLeft();
      drawScore();
    }
    else if (move == 1) {
      moveRight();
      drawScore();
    }
    else if (move == 2) {
      rotateTetromino();
      drawScore();
    }
    */

 } else {
    // Game Over screen
    tft.fillScreen(BLACK);
    tft.setCursor(20, 50);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.print("Game Over!");
    tft.setCursor(20, 70);
    tft.print("Score: ");
    tft.print(score);

    // Wait until any button is pressed to restart
    while (true) {
        // Check if any button is pressed
        if (digitalRead(buttonLeftPin) == LOW || digitalRead(buttonRightPin) == LOW || 
            digitalRead(buttonRotatePin) == LOW || digitalRead(buttonDownPin) == LOW) {
            delay(200); // Debounce delay to avoid accidental multiple restarts
            initGame();
            break;  // Exit the loop and restart the game
        }
        delay(10);  // Small delay to prevent excessive CPU usage
    }
}
}