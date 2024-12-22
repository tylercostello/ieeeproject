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
#define BALL_COLOR 0x001F  // Blue
#define PADDLE_COLOR 0xF800  // Red
#define SCORE_COLOR WHITE

extern Adafruit_SSD1351 tft; // Use the shared display object from the main menu
extern const int buttonUpPin, buttonDownPin, buttonLeftPin, buttonRightPin, buttonMenuPin;

// Pong game variables
int ballX, ballY;       // Ball position
int ballSpeedX = 2;     // Ball speed in X direction
int ballSpeedY = 2;     // Ball speed in Y direction
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
  randomSeed(analogRead(0));
  ballX = SCREEN_WIDTH / 2;
  ballY = SCREEN_HEIGHT / 2;
  leftPaddleY = SCREEN_HEIGHT / 2 - paddleHeight / 2;
  rightPaddleY = SCREEN_HEIGHT / 2 - paddleHeight / 2;
  drawPongScore();
}

// Function to draw the paddles and ball
void drawPaddle(int x, int y, uint16_t color) {
  tft.fillRect(x, y, paddleWidth, paddleHeight, color);
}

void drawBall(int x, int y, uint16_t color) {
  tft.fillCircle(x, y, ballRadius, color);
}

void drawPongScore() {
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

  // move paddle down if buttondownpin on
  // else move paddle up if buttonuppin on 
    if (digitalRead(buttonDownPin) == LOW) {
        leftPaddleY += paddleSpeed;
        Serial.println("left paddle down");
    } else if (digitalRead(buttonUpPin) == LOW) {
        leftPaddleY -= paddleSpeed;
        Serial.println("left paddle up");
    }

  // AI for the right paddle (follows the ball with slight random offset)
  if (ballY < rightPaddleY + paddleHeight / 2) {
    rightPaddleY -= paddleSpeed;
  } else if (ballY > rightPaddleY + paddleHeight / 2) {
    rightPaddleY += paddleSpeed;
  }

  // Prevent paddles from going off-screen
  if (leftPaddleY < 20) leftPaddleY = 20;
  if (leftPaddleY > SCREEN_HEIGHT - paddleHeight) leftPaddleY = SCREEN_HEIGHT - paddleHeight;
  if (rightPaddleY < 20) rightPaddleY = 20;
  if (rightPaddleY > SCREEN_HEIGHT - paddleHeight) rightPaddleY = SCREEN_HEIGHT - paddleHeight;
}
void updateBall() {
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Bounce the ball off the top and bottom
  if (ballY <= ballRadius+15 || ballY >= SCREEN_HEIGHT - ballRadius) {
    ballSpeedY = -ballSpeedY;
  }

  // Check for collisions with the left paddle
  if (ballX - ballRadius <= leftPaddleX + paddleWidth && ballY >= leftPaddleY && ballY <= leftPaddleY + paddleHeight) {
    ballSpeedX = abs(ballSpeedX);  // Ensure ball moves to the right
    ballSpeedY = random(-4, 4);  // Add some randomness to the bounce
    ballX = leftPaddleX + paddleWidth + ballRadius;  // Prevent the ball from getting stuck
  }

  // Check for collisions with the right paddle
  if (ballX + ballRadius >= rightPaddleX && ballY >= rightPaddleY && ballY <= rightPaddleY + paddleHeight) {
    ballSpeedX = -abs(ballSpeedX);  // Ensure ball moves to the left
    ballSpeedY = random(-4, 4);  // Add some randomness to the bounce
    ballX = rightPaddleX - ballRadius;  // Prevent the ball from getting stuck
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


void runPongGame(){
    initGame();
    while (true){
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
        // Exit to menu on button press
        if (digitalRead(buttonMenuPin) == LOW) {
          break;
        }

        // Delay to control the frame rate
        delay(20);

    }

}
