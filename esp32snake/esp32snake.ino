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

const int buttonLeftPin = 12;
const int buttonRightPin = 14;
const int buttonUpPin = 32;
const int buttonDownPin = 33;
const int buttonMenuPin = 35;

#define BLACK 0x0000
#define WHITE 0xFFFF
#define SNAKE_COLOR 0x07E0 // Green
#define FOOD_COLOR 0xF800  // Red
#define SCORE_COLOR WHITE

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

const int snakeSize = 4;
int snakeLength = 3;          // Initial snake length
int snakeX[100], snakeY[100]; // Max snake length is 100 segments

int foodX, foodY; // Food position

int direction = 2; // 0=up, 1=down, 2=right, 3=left
int score = 0;     // Player score

void initGame()
{
    // Initialize snake starting position
    for (int i = 0; i < 100; i++)
    {
        snakeX[i] = 0;
        snakeY[i] = 0;
    }
    snakeX[0] = SCREEN_WIDTH / 2;
    snakeY[0] = SCREEN_HEIGHT / 2;
    snakeX[1] = snakeX[0] - snakeSize;
    snakeY[1] = snakeY[0];
    snakeX[2] = snakeX[1] - snakeSize;
    snakeY[2] = snakeY[1];
    snakeLength = 3;
    direction = 2;

    // Spawn initial food
    spawnFood();

    // Reset score
    score = 0;
    drawScore();
}

void spawnFood()
{
    foodX = random(0, SCREEN_WIDTH / snakeSize) * snakeSize;
    foodY = random(20, SCREEN_HEIGHT / snakeSize) * snakeSize;
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
    // Shift body
    for (int i = snakeLength - 1; i > 0; i--)
    {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    // Move head based on AI decision
    // aiMoveTowardFood();
    buttonMoveTowardFood();

    // Check for wall collision
    if (snakeX[0] < 0 || snakeX[0] >= SCREEN_WIDTH || snakeY[0] < 20 || snakeY[0] >= SCREEN_HEIGHT)
    {
        // Game over, restart
        initGame();
        return;
    }
}

void buttonMoveTowardFood()
{

    if (digitalRead(buttonRightPin) == LOW && direction != 3)
    {
        direction = 2; // Move right
    }
    else if (digitalRead(buttonLeftPin) == LOW && direction != 2)
    {
        direction = 3; // Move left
    }
    else if (digitalRead(buttonDownPin) == LOW && direction != 0)
    {
        direction = 1; // Move down
    }
    else if (digitalRead(buttonUpPin) == LOW && direction != 1)
    {
        direction = 0; // Move up
    }

    // Move head based on the current direction
    switch (direction)
    {
    case 0:
        snakeY[0] -= snakeSize;
        break; // Move up
    case 1:
        snakeY[0] += snakeSize;
        break; // Move down
    case 2:
        snakeX[0] += snakeSize;
        break; // Move right
    case 3:
        snakeX[0] -= snakeSize;
        break; // Move left
    }
}


void checkFoodCollision()
{
    if (snakeX[0] == foodX && snakeY[0] == foodY)
    {
        // Increase snake length properly to avoid glitches
        snakeLength++;

        // Set new tail at the old position of the last segment
        snakeX[snakeLength - 1] = snakeX[snakeLength - 2];
        snakeY[snakeLength - 1] = snakeY[snakeLength - 2];

        // Increase score
        score++;

        // Spawn new food
        spawnFood();

        // Update score display
        drawScore();
    }
}

void drawScore()
{
    tft.fillRect(0, 0, SCREEN_WIDTH, 15, BLACK); // Clear score area
    tft.setCursor(5, 5);
    tft.setTextColor(SCORE_COLOR);
    tft.setTextSize(1);
    tft.print("Score: ");
    tft.print(score);
}

void checkSelfCollision()
{
    for (int i = 1; i < snakeLength; i++)
    {
        if (snakeX[0] == snakeX[i] && snakeY[0] == snakeY[i])
        {
            // Game over, restart
            initGame();
            break;
        }
    }
}

void setup()
{
    pinMode(buttonLeftPin, INPUT_PULLUP);
    pinMode(buttonRightPin, INPUT_PULLUP);
    pinMode(buttonUpPin, INPUT_PULLUP);
    pinMode(buttonDownPin, INPUT_PULLUP);
    pinMode(buttonMenuPin, INPUT_PULLUP);
    Serial.begin(9600);
    tft.begin();
    tft.fillScreen(BLACK);
    randomSeed(analogRead(0));

    initGame();
}

void loop()
{
    // Erase previous snake and food positions
    for (int i = 0; i < snakeLength; i++)
    {
        tft.fillRect(snakeX[i], snakeY[i], snakeSize, snakeSize, BLACK);
    }
    tft.fillRect(foodX, foodY, snakeSize, snakeSize, BLACK);

    // Move snake and update game state
    moveSnake();
    checkFoodCollision();
    checkSelfCollision();

    // Redraw snake and food
    drawSnake();
    drawFood();

    // Delay to control the frame rate
    delay(100);
}
