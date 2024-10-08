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
#define BLUE 0x001F

Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, CS_PIN, DC_PIN, RST_PIN);

const int ballCount = 3; // Number of balls
int ballRadius = 5;

// Arrays to store ball data
int ballX[ballCount];
int ballY[ballCount];
int ballSpeedX[ballCount];
int ballSpeedY[ballCount];
uint16_t ballColor[ballCount];

// Function to initialize the balls with random positions and speeds
void initBalls() {
  for (int i = 0; i < ballCount; i++) {
    ballX[i] = random(ballRadius, SCREEN_WIDTH - ballRadius);
    ballY[i] = random(ballRadius, SCREEN_HEIGHT - ballRadius);
    ballSpeedX[i] = random(1, 3) * (random(2) == 0 ? 1 : -1); // Random speed between -3 and 3
    ballSpeedY[i] = random(1, 3) * (random(2) == 0 ? 1 : -1);
    ballColor[i] = BLUE; // You can randomize the color for each ball if you want
  }
}

// Function to check if two balls are colliding
bool isColliding(int i, int j) {
  int dx = ballX[i] - ballX[j];
  int dy = ballY[i] - ballY[j];
  int distanceSquared = dx * dx + dy * dy;
  int radiusSum = 2 * ballRadius; // Since both balls have the same radius
  return distanceSquared <= radiusSum * radiusSum;
}

void setup(void) {
  Serial.begin(9600);
  tft.begin();
  tft.fillScreen(BLACK);
  randomSeed(analogRead(0)); // Seed for random number generation

  // Initialize the balls
  initBalls();
}

void loop() {
  // Clear the previous ball positions
  for (int i = 0; i < ballCount; i++) {
    tft.fillCircle(ballX[i], ballY[i], ballRadius, BLACK);
  }

  // Update positions and check for collisions with walls
  for (int i = 0; i < ballCount; i++) {
    ballX[i] += ballSpeedX[i];
    ballY[i] += ballSpeedY[i];

    // Bounce off the walls
    if (ballX[i] <= ballRadius || ballX[i] >= SCREEN_WIDTH - ballRadius) {
      ballSpeedX[i] = -ballSpeedX[i];
    }
    if (ballY[i] <= ballRadius || ballY[i] >= SCREEN_HEIGHT - ballRadius) {
      ballSpeedY[i] = -ballSpeedY[i];
    }

    // Check for collisions with other balls
    for (int j = i + 1; j < ballCount; j++) {
      if (isColliding(i, j)) {
        // Reverse directions for both balls
        ballSpeedX[i] = -ballSpeedX[i];
        ballSpeedY[i] = -ballSpeedY[i];
        ballSpeedX[j] = -ballSpeedX[j];
        ballSpeedY[j] = -ballSpeedY[j];
      }
    }
  }

  // Draw the balls in their new positions
  for (int i = 0; i < ballCount; i++) {
    tft.fillCircle(ballX[i], ballY[i], ballRadius, ballColor[i]);
  }

  // Delay to control the speed of the animation
  delay(30);
}
