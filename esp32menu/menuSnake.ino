#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <EEPROM.h>

extern Adafruit_SSD1351 tft; // Use the shared display object from the main menu
extern const int buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonMenuPin;
extern void checkPauseMenu(void (*resetGame)(), void (*exitGame)());

#define BLACK 0x0000
#define WHITE 0xFFFF
#define SNAKE_COLOR 0x07E0 // Green
#define FOOD_COLOR 0xF800  // Red
#define SCORE_COLOR WHITE

const int highScoreSnakeAddress = 0; // Starting address in EEPROM
int highScoreSnake = 0; // Variable to store the high score

const int snakeSize = 4;
int snakeLength = 3;          // Initial snake length
int snakeX[100], snakeY[100]; // Max snake length is 100 segments

int foodX, foodY;  // Food position
int direction = 2; // 0=up, 1=down, 2=right, 3=left
int snakeScore = 0;     // Player score

int storedDirection = 2; 
bool gameOverSnake = false;
bool exitGameBoolSnake = false;

int newDirection = 2; // 0=up, 1=down, 2=right, 3=left



// pause menu delay
void pauseDelaySnake(int ms)
{
    unsigned long start = millis();
    while (millis() - start < ms)
    {
        buttonMove(); 
        // check for button press to exit
        if (digitalRead(buttonMenuPin) == LOW)
        {
            // call pause menu script here
            
            checkPauseMenu(initSnakeGame, exitGameSnake);
            if (gameOverSnake){
                Serial.begin(115200);
                Serial.print("Loaded High Score: ");
                Serial.println(highScoreSnake);
                tft.fillScreen(BLACK); // Clear the screen
                tft.setCursor(10, tft.height() / 2 - 30);
                tft.setTextColor(WHITE);
                tft.setTextSize(2);
                tft.println("Game Over!");

                tft.setCursor(10, tft.height() / 2 - 10);
                tft.setTextSize(1);
                tft.print("Score: ");
                tft.print(snakeScore);

                tft.setCursor(10, tft.height() / 2 + 10);
                tft.print("High Score: ");

                tft.print(highScoreSnake);
                if (snakeScore > EEPROM.get(highScoreSnakeAddress, highScoreSnake))
                {
                    highScoreSnake = snakeScore;
                    EEPROM.put(highScoreSnakeAddress, highScoreSnake); // Save new high score to EEPROM
                    EEPROM.commit(); // Save changes to flash memory

                    tft.setCursor(10, tft.height() / 2 + 30);
                    tft.print("New High Score!");
                }
            }
            else{
                tft.drawRect(0, 20, tft.width(), tft.height() - 20, WHITE);
                drawSnakeScore();
            }
            
        }
    }
}

void exitGameSnake()
{
    EEPROM.commit();  // Ensure any changes to EEPROM are saved before exiting
    exitGameBoolSnake = true;
    gameOverSnake = false;
}


void displayGameOver()
{
    gameOverSnake = true;
    tft.fillScreen(BLACK); // Clear the screen
    tft.setCursor(0, tft.height() / 2 - 30);
    tft.setTextColor(WHITE);
    tft.setTextSize(2);
    tft.println("Game Over!");

    tft.setCursor(10, tft.height() / 2 - 10);
    tft.setTextSize(1);
    tft.print("Score: ");
    tft.print(snakeScore);

    tft.setCursor(10, tft.height() / 2 + 10);
    tft.print("High Score: ");
    tft.print(highScoreSnake);

    // Check if the current score is a new high score
    if (snakeScore > EEPROM.get(highScoreSnakeAddress, highScoreSnake))
    {
        highScoreSnake = snakeScore;
        EEPROM.put(highScoreSnakeAddress, highScoreSnake); // Save new high score to EEPROM
        EEPROM.commit(); // Save changes to flash memory

        tft.setCursor(10, tft.height() / 2 + 30);
        tft.print("New High Score!");
    }


    pauseDelaySnake(100); 
    
    while (gameOverSnake) {
        // wait until any button is pressed
        if (digitalRead(buttonUpPin) == LOW || digitalRead(buttonDownPin) == LOW || digitalRead(buttonLeftPin) == LOW || digitalRead(buttonRightPin) == LOW){
            break;
        }
        pauseDelaySnake(10); 
    }
    if (exitGameBoolSnake){
        return;
    }
    gameOverSnake = false;
    initSnakeGame(); // Restart the game
}

void initSnakeGame()
{
    EEPROM.get(highScoreSnakeAddress, highScoreSnake);
    exitGameBoolSnake = false;
    gameOverSnake = false;
    // Clear the screen
    tft.fillScreen(BLACK);

    // Draw white border
    tft.drawRect(0, 20, tft.width(), tft.height() - 20, WHITE);

    // Initialize snake starting position
    for (int i = 0; i < 100; i++)
    {
        snakeX[i] = 0;
        snakeY[i] = 0;
    }
    snakeX[0] = tft.width() / 2;
    snakeY[0] = tft.height() / 2;
    snakeX[1] = snakeX[0] - snakeSize;
    snakeY[1] = snakeY[0];
    snakeX[2] = snakeX[1] - snakeSize;
    snakeY[2] = snakeY[1];
    snakeLength = 3;
    direction = 2;
    storedDirection = 2;
    newDirection = 2;

    // Spawn initial food
    spawnFood();

    // Reset score
    snakeScore = 0;
    drawSnakeScore();
}

bool isFoodOnSnake(int x, int y)
{
    for (int i = 0; i < snakeLength; i++)
    {
        if (snakeX[i] == x && snakeY[i] == y)
        {
            return true;
        }
    }
    return false;
}

void spawnFood()
{
    const int borderThickness = snakeSize; // Thickness of the white border

    do
    {
        foodX = random(borderThickness, tft.width() - borderThickness) / snakeSize * snakeSize;
        foodY = random(20 + borderThickness, tft.height() - borderThickness) / snakeSize * snakeSize;
    } while (isFoodOnSnake(foodX, foodY));
}

void drawSnake()
{
    for (int i = 0; i < snakeLength; i++)
    {
        tft.fillRect(snakeX[i], snakeY[i], snakeSize, snakeSize, SNAKE_COLOR);
    }
}

void drawFood()
{
    tft.fillRect(foodX, foodY, snakeSize, snakeSize, FOOD_COLOR);
}

void moveSnake()
{
    direction = storedDirection; // Apply new direction only at the start of movement

    // Shift body
    for (int i = snakeLength - 1; i > 0; i--)
    {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    // Move head
    switch (direction)
    {
    case 0:
        snakeY[0] -= snakeSize;
        break;
    case 1:
        snakeY[0] += snakeSize;
        break;
    case 2:
        snakeX[0] += snakeSize;
        break;
    case 3:
        snakeX[0] -= snakeSize;
        break;
    }

    // Check for wall collision
    if (snakeX[0] < snakeSize || snakeX[0] >= tft.width() - snakeSize ||
        snakeY[0] < 20 + snakeSize || snakeY[0] >= tft.height() - snakeSize)
    {
        displayGameOver();
    }
}

void buttonMove()
{
    int newDirection = direction; // Keep the current direction unless a valid change is made

    if (digitalRead(buttonRightPin) == LOW && direction != 3)
    {
        newDirection = 2; // Move right
    }
    else if (digitalRead(buttonLeftPin) == LOW && direction != 2)
    {
        newDirection = 3; // Move left
    }
    else if (digitalRead(buttonDownPin) == LOW && direction != 0)
    {
        newDirection = 1; // Move down
    }
    else if (digitalRead(buttonUpPin) == LOW && direction != 1)
    {
        newDirection = 0; // Move up
    }

    // Only update direction if it's different from the current one
    if (newDirection != direction)
    {
        storedDirection = newDirection;
    }
}



void checkFoodCollision()
{
    if (snakeX[0] == foodX && snakeY[0] == foodY)
    {
        // Increase snake length properly
        snakeLength++;

        // Update new tail segment
        snakeX[snakeLength - 1] = snakeX[snakeLength - 2];
        snakeY[snakeLength - 1] = snakeY[snakeLength - 2];

        // Increase score and spawn new food
        snakeScore++;
        spawnFood();
        drawSnakeScore();
    }
}

void drawSnakeScore()
{
    tft.fillRect(0, 0, tft.width(), 15, BLACK); // Clear score area
    tft.setCursor(40, 5);
    tft.setTextColor(SCORE_COLOR);
    tft.setTextSize(1);
    tft.print("Score: ");
    tft.print(snakeScore);
}

void checkSelfCollision()
{
    for (int i = 1; i < snakeLength; i++)
    {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
        {
            displayGameOver(); // Display game over message
            break;
        }
    }
}

void runSnakeGame()
{
    initSnakeGame(); // Initialize the game state
    while (!exitGameBoolSnake)
    {
        // Clear previous positions
        for (int i = 0; i < snakeLength; i++)
        {
            tft.fillRect(snakeX[i], snakeY[i], snakeSize, snakeSize, BLACK);
        }
        tft.fillRect(foodX, foodY, snakeSize, snakeSize, BLACK);

        // Update game state
        buttonMove();
        moveSnake();
        checkFoodCollision();
        checkSelfCollision();

        // Draw updated positions
        drawSnake();
        drawFood();

        pauseDelaySnake(120); // Control game speed
    }
}
