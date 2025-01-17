#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

extern Adafruit_SSD1351 tft; // Use the shared display object from the main menu
extern const int buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonMenuPin;
extern void checkPauseMenu(void (*resetGame)(), void (*exitGame)());
const int buttonRotatePin = buttonUpPin;

#define BLACK 0x0000
#define WHITE 0xFFFF
#define GRAY 0x4208
#define BACKGROUND_COLOR 0x0000 // Black

// Variables to store button states
int buttonLeftState = 0;
int buttonRightState = 0;
int buttonRotateState = 0;
int buttonDownState = 0;

int lastButtonLeftState = HIGH;
int lastButtonRightState = HIGH;
int lastButtonRotateState = HIGH;
int lastButtonDownState = HIGH;

bool lossScreenTetris = false;

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
int prevTetromino[4][4] = {0}; // Previous tetromino shape
int tetrisScore = 0;
bool gameOverTetris = false;
uint16_t currentColor; // Color of the current tetromino
const int tetrisScores[4] = {40, 100, 300, 1200};

int tetrisDelay = 501;
int totalLinesCleared = 0;

bool exitGameBoolTetris = false;

// Tetromino shapes (I, O, T, L, J, Z, S)
const int tetrominoShapes[7][4][4] = {
    {{1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // I
    {{1, 1, 0, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // O
    {{0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // T
    {{1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // L
    {{0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // J
    {{1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}, // Z
    {{0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}}  // S
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

int nextTetromino[4][4] = {0};
uint16_t nextColor;

// Function prototypes
void initTetrisGame();
void spawnTetromino();
void drawGrid();
void moveDown();
void moveLeft();
void moveRight();
void rotateTetromino();
bool checkCollision();
void lockTetromino();
void checkLines();
void drawTetrisScore();



void pauseDelayTetris(int ms)
{
    unsigned long start = millis();
    while (millis() - start < ms)
    {
        // can add button checks here as well
        // check for button press to exit
        if (digitalRead(buttonMenuPin) == LOW)
        {
            // call pause menu script here
            checkPauseMenu(initTetrisGame, exitGameTetris);
            // extra drawing if needed
            if (lossScreenTetris)
            {
                tft.fillScreen(BLACK);
                tft.setCursor(5, 50);
                tft.setTextColor(WHITE);
                tft.setTextSize(2);
                tft.print("Game Over!");
                tft.setCursor(0, 70);
                tft.print("Score: ");
                tft.setCursor(0, 90);
                tft.print(tetrisScore);
            }
            else
            {
                drawNextTetromino();
            }
            
        }
    }
}
void exitGameTetris()
{
    gameOverTetris = false;
    exitGameBoolTetris = true;
}


// Function to initialize the game
void initTetrisGame()
{
    exitGameBoolTetris = false;
    lossScreenTetris = false;
     //use current micros to get a random seed
    randomSeed(micros());
    // Clear the grid
    for (int y = 0; y < GRID_HEIGHT; y++)
    {
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            grid[y][x] = 0;
        }
    }

    tetrisScore = 0;
    gameOverTetris = false;

    tft.fillScreen(BLACK);

    // Initialize the next Tetromino
    int shapeIndex = random(0, 7);
    nextColor = tetrominoColors[shapeIndex];
    memset(nextTetromino, 0, sizeof(nextTetromino));
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            nextTetromino[y][x] = tetrominoShapes[shapeIndex][y][x];
        }
    }

    spawnTetromino();
    drawGrid();
    drawTetrisScore();
    drawNextTetromino();
}
// Function to spawn a new tetromino
void spawnTetromino()
{
    // Copy nextTetromino as the new current Tetromino
    memcpy(currentTetromino, nextTetromino, sizeof(currentTetromino));
    currentColor = nextColor;

    tetrominoX = 3;
    tetrominoY = 0;

    // Pick a new random tetromino shape for the next preview
    int shapeIndex = random(0, 7);
    nextColor = tetrominoColors[shapeIndex];

    memset(nextTetromino, 0, sizeof(nextTetromino));
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            nextTetromino[y][x] = tetrominoShapes[shapeIndex][y][x];
        }
    }

    // Draw the next Tetromino on the side
    drawNextTetromino();
}
void drawNextTetromino()
{
    // Clear the preview area
    tft.fillRect(80, 40, 40, 40, BLACK); // Adjust position and size as needed

    // Draw the next Tetromino blocks in a 4x4 grid
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (nextTetromino[y][x] == 1)
            {
                int screenX = 80 + x * BLOCK_SIZE; // Adjust X offset for preview area
                int screenY = 50 + y * BLOCK_SIZE; // Adjust Y offset for preview area
                tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, nextColor);
            }
        }
    }

    // Optional: Label the preview area
    tft.setCursor(80, 35);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);
    tft.print("Next:");
}

// Function to draw the grid and tetrominoes
// Function to draw the grid, border, and tetrominoes
void drawGrid()
{
    // Clear previous tetromino position
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (prevTetromino[y][x] == 1)
            {
                int screenX = (prevTetrominoX + x) * BLOCK_SIZE;
                int screenY = (prevTetrominoY + y) * BLOCK_SIZE;
                if (prevTetrominoY + y < GRID_HEIGHT && prevTetrominoX + x < GRID_WIDTH &&
                    grid[prevTetrominoY + y][prevTetrominoX + x] == 0)
                {
                    tft.fillRect(screenX, screenY, BLOCK_SIZE, BLOCK_SIZE, BLACK);
                }
            }
        }
    }

    // Draw grid pattern
    for (int x = 0; x <= GRID_WIDTH; x++)
    {
        int lineX = x * BLOCK_SIZE;
        tft.drawFastVLine(lineX, 0, GRID_HEIGHT * BLOCK_SIZE, GRAY); // Vertical grid lines
    }
    for (int y = 0; y <= GRID_HEIGHT; y++)
    {
        int lineY = y * BLOCK_SIZE;
        tft.drawFastHLine(0, lineY, GRID_WIDTH * BLOCK_SIZE, GRAY); // Horizontal grid lines
    }

    // Right border
    tft.drawFastVLine(GRID_WIDTH * BLOCK_SIZE, 0, GRID_HEIGHT * BLOCK_SIZE, GRAY);

    // Draw current tetromino
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (currentTetromino[y][x] == 1)
            {
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
void moveDown()
{
    tetrominoY++;

    if (checkCollision())
    {
        tetrominoY--;
        lockTetromino();
        spawnTetromino();
        if (checkCollision())
        {
            gameOverTetris = true;
        }
    }

    drawGrid();
}

// Function to move the tetromino left
void moveLeft()
{
    tetrominoX--;
    if (checkCollision())
        tetrominoX++;
    drawGrid();
}

// Function to move the tetromino right
void moveRight()
{
    tetrominoX++;
    if (checkCollision())
        tetrominoX--;
    drawGrid();
}

// Function to rotate the tetromino
void rotateTetromino()
{
    int temp[4][4] = {0};

    // Perform rotation
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
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
    if (checkCollision())
    {
        // Revert if invalid
        tetrominoX = tempX;
        tetrominoY = tempY;
        memcpy(currentTetromino, tempShape, sizeof(currentTetromino));
    }
    else
    {
        drawGrid();
    }
}

// Function to check for collisions
bool checkCollision()
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (currentTetromino[y][x] == 1)
            {
                int newX = tetrominoX + x;
                int newY = tetrominoY + y;

                // Check for out-of-bounds or collision with other blocks
                if (newX < 0 || newX >= GRID_WIDTH || newY >= GRID_HEIGHT ||
                    (newY >= 0 && grid[newY][newX] == 1))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

// Function to lock the tetromino in place
void lockTetromino()
{
    for (int y = 0; y < 4; y++)
    {
        for (int x = 0; x < 4; x++)
        {
            if (currentTetromino[y][x] == 1)
            {
                int gridY = tetrominoY + y;
                int gridX = tetrominoX + x;
                if (gridY >= 0 && gridY < GRID_HEIGHT && gridX >= 0 && gridX < GRID_WIDTH)
                {
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
void checkLines()
{
    int linesCleared = 0;
    for (int y = GRID_HEIGHT - 1; y >= 0; y--)
    {
        bool fullLine = true;

        // Check if line is full
        for (int x = 0; x < GRID_WIDTH; x++)
        {
            if (grid[y][x] == 0)
            {
                fullLine = false;
                break;
            }
        }

        if (fullLine)
        {
            // Clear the line visually
            tft.fillRect(0, y * BLOCK_SIZE, GRID_WIDTH * BLOCK_SIZE, BLOCK_SIZE, BLACK);

            // Shift everything down
            for (int newY = y; newY > 0; newY--)
            {
                for (int x = 0; x < GRID_WIDTH; x++)
                {
                    grid[newY][x] = grid[newY - 1][x];
                    // Update the display
                    if (grid[newY][x] == 1)
                    {
                        tft.fillRect(x * BLOCK_SIZE, newY * BLOCK_SIZE,
                                     BLOCK_SIZE, BLOCK_SIZE, currentColor);
                    }
                    else
                    {
                        tft.fillRect(x * BLOCK_SIZE, newY * BLOCK_SIZE,
                                     BLOCK_SIZE, BLOCK_SIZE, BLACK);
                    }
                }
            }

            // Clear the top line
            for (int x = 0; x < GRID_WIDTH; x++)
            {
                grid[0][x] = 0;
            }



            // Check the same line again as lines might have shifted down
            linesCleared++;
            totalLinesCleared++;
            y++;
        }
    }

    int level = totalLinesCleared / 10;
    tetrisScore = tetrisScore + tetrisScores[linesCleared - 1] * (level + 1);
    tetrisDelay = max(50, 500 - level * 50);
    drawTetrisScore();
}

// Function to draw the score
void drawTetrisScore()
{
    tft.fillRect(65, 5, SCREEN_WIDTH, 30, BLACK); // Clear score area
    tft.setCursor(80, 5);
    tft.setTextColor(WHITE);
    tft.setTextSize(1);

    tft.print("Score: ");
    tft.setCursor(80, 15);
 

    tft.print(tetrisScore);
}

void runTetrisGame()
{
    initTetrisGame(); // Initialize the game state
    while (!exitGameBoolTetris)
    {
        if (!gameOverTetris)
        {
            // Move the tetromino down at regular intervals
            static unsigned long lastUpdateTime = 0;
            unsigned long currentTime = millis();

            if (currentTime - lastUpdateTime >= tetrisDelay)
            { // Move down every 500ms
                moveDown();
                drawTetrisScore();
                lastUpdateTime = currentTime;
            }

            // Check for button inputs
            // Replace these with your actual button pins and logic
            buttonLeftState = digitalRead(buttonLeftPin);
            buttonRightState = digitalRead(buttonRightPin);
            buttonRotateState = digitalRead(buttonRotatePin);
            buttonDownState = digitalRead(buttonDownPin);

            if (buttonLeftState == LOW && lastButtonLeftState == HIGH)
            {
                Serial.println("Left button pressed");
                moveLeft();
                drawTetrisScore();
            }

            // Check if the Right button was pressed (HIGH to LOW transition)
            if (buttonRightState == LOW && lastButtonRightState == HIGH)
            {
                Serial.println("Right button pressed");
                moveRight();
                drawTetrisScore();
            }

            // Check if the Rotate button was pressed (HIGH to LOW transition)
            if (buttonRotateState == LOW && lastButtonRotateState == HIGH)
            {
                Serial.println("Rotate button pressed");
                rotateTetromino();
                drawTetrisScore();
            }

            // Check if the Down button was pressed (HIGH to LOW transition)
            if (buttonDownState == LOW && lastButtonDownState == HIGH)
            {
                Serial.println("Down button pressed");
                moveDown(); // Assuming you have a function for moving down
                drawTetrisScore();
            }

            // Update the previous button states for the next loop
            lastButtonLeftState = buttonLeftState;
            lastButtonRightState = buttonRightState;
            lastButtonRotateState = buttonRotateState;
            lastButtonDownState = buttonDownState;
            pauseDelayTetris(50);
        }
        else
        {
            lossScreenTetris = true;
            // Game Over screen
            tft.fillScreen(BLACK);
            tft.setCursor(5, 50);
            tft.setTextColor(WHITE);
            tft.setTextSize(2);
            tft.print("Game Over!");
            tft.setCursor(0, 70);
            tft.print("Score: ");
            tft.setCursor(0, 90);
            tft.print(tetrisScore);

            // Wait until any button is pressed to restart
            while (lossScreenTetris)
            {
                // Check if any button is pressed
                if (digitalRead(buttonLeftPin) == LOW || digitalRead(buttonRightPin) == LOW ||
                    digitalRead(buttonRotatePin) == LOW || digitalRead(buttonDownPin) == LOW)
                {
                    pauseDelayTetris(200); // Debounce pauseDelayTetris to avoid accidental multiple restarts
                    initTetrisGame();
                    break; // Exit the loop and restart the game
                }
                pauseDelayTetris(10); // Small delay to prevent excessive CPU usage
                if (exitGameBoolTetris)
                {
                    break;
                }
            }

            lossScreenTetris = false;
        }

    }

    
}
