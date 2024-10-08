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
#define BALL_COLOR 0x001F  // Blue
#define PADDLE_COLOR 0xF800  // Red
#define SCORE_COLOR WHITE

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

// Pong game variables
int ballX, ballY;       // Ball position
int ballSpeedX = 3;     // Ball speed in X direction
int ballSpeedY = 5;     // Ball speed in Y direction
int ballRadius = 3;     // Ball size

// Paddles
int paddleWidth = 2;
int paddleHeight = 20;
int paddleSpeed = 3;
int leftPaddleY, rightPaddleY; // Paddles' Y positions

// Game boundaries
int leftPaddleX = 5;           // Left paddle X position
int rightPaddleX = SCREEN_WIDTH - leftPaddleX - paddleWidth; // Right paddle X position

// Score
int leftScore = 0;
int rightScore = 0;

// Initialize the ball and paddles
void initGame() {
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  leftPaddleY = SCREEN_HEIGHT / 2 - paddleHeight / 2;
  rightPaddleY = SCREEN_HEIGHT / 2 - paddleHeight / 2;
}

// Function to draw the paddles and ball
void drawPaddle(int x, int y, uint16_t color) {
  tft.fillRect(x, y, paddleWidth, paddleHeight, color);
}

void drawBall(int x, int y, uint16_t color) {
  tft.fillCircle(x, y, ballRadius, color);
}

void drawScore() {
  // Clear previous score display without affecting the rest of the screen
  tft.fillRect(70, 5, 50, 10, BLACK);

  tft.setCursor(30, 5);
  tft.setTextColor(WHITE);
  tft.setTextSize(1);
  tft.print("Score: ");
  tft.print(leftScore);
  tft.print(" - ");
  tft.print(rightScore);
}

// AI for controlling paddles
void movePaddles() {
  // AI for the left paddle (follows the ball with slight random offset)
  if (ballY < leftPaddleY + paddleHeight / 2) {
    leftPaddleY -= paddleSpeed;
  } else if (ballY > leftPaddleY + paddleHeight / 2) {
    leftPaddleY += paddleSpeed;
  }

  // AI for the right paddle (follows the ball with slight random offset)
  if (ballY < rightPaddleY + paddleHeight / 2) {
    rightPaddleY -= paddleSpeed;
  } else if (ballY > rightPaddleY + paddleHeight / 2) {
    rightPaddleY += paddleSpeed;
  }

  // Prevent paddles from going off-screen
  if (leftPaddleY < 0) leftPaddleY = 0;
  if (leftPaddleY > SCREEN_HEIGHT - paddleHeight) leftPaddleY = SCREEN_HEIGHT - paddleHeight;
  if (rightPaddleY < 0) rightPaddleY = 0;
  if (rightPaddleY > SCREEN_HEIGHT - paddleHeight) rightPaddleY = SCREEN_HEIGHT - paddleHeight;
}

void updateBall() {
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Bounce the ball off the top and bottom
  if (ballY <= ballRadius + 15 || ballY >= SCREEN_HEIGHT - ballRadius) {
    ballSpeedY = -ballSpeedY;
  }

  // Check for collisions with the paddles and adjust the bounce angle
  if (ballX - ballRadius <= leftPaddleX + paddleWidth && ballY >= leftPaddleY && ballY <= leftPaddleY + paddleHeight) {
    ballSpeedX = -ballSpeedX;  // Ball bounces off left paddle
    float hitPos = (ballY - leftPaddleY) - (paddleHeight / 2);  // Distance from center
    ballSpeedY = hitPos / (paddleHeight / 2) * 5;  // Adjust Y speed based on hit position
  }
  if (ballX + ballRadius >= rightPaddleX && ballY >= rightPaddleY && ballY <= rightPaddleY + paddleHeight) {
    ballSpeedX = -ballSpeedX;  // Ball bounces off right paddle
    float hitPos = (ballY - rightPaddleY) - (paddleHeight / 2);  // Distance from center
    ballSpeedY = hitPos / (paddleHeight / 2) * 5;  // Adjust Y speed based on hit position
  }

  // Check if the ball goes out of bounds
  if (ballX <= 0) {
    rightScore++;  // Right player scores
    initGame();    // Reset ball and paddles
  } else if (ballX >= SCREEN_WIDTH) {
    leftScore++;   // Left player scores
    initGame();    // Reset ball and paddles
  }
}

void setup() {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(BLACK);
  randomSeed(analogRead(0));

  initGame();
  drawScore();  // Draw the initial score, which stays on the screen
}

void loop() {
  // Erase only the previous positions of the ball and paddles
  tft.fillRect(leftPaddleX, leftPaddleY, paddleWidth, paddleHeight, BLACK);
  tft.fillRect(rightPaddleX, rightPaddleY, paddleWidth, paddleHeight, BLACK);
  tft.fillCircle(ballX, ballY, ballRadius, BLACK);

  // Update positions
  movePaddles();
  updateBall();

  // Draw new positions of the paddles and ball
  drawPaddle(leftPaddleX, leftPaddleY, PADDLE_COLOR);
  drawPaddle(rightPaddleX, rightPaddleY, PADDLE_COLOR);
  drawBall(ballX, ballY, BALL_COLOR);

  // Redraw the score without flickering (since it doesn't change frequently)
  drawScore();

  // Delay to control the frame rate
  delay(20);
}
