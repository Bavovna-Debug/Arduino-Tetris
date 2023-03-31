#define INPUT_KEYS    // Compile keys handler.
#define INPUT_GYRO    // Compile motion controller.
#undef OUTPUT_MAX     // Compile LED matrix as output.
#define OUTPUT_TFT    // Compile LCD display as output.
#define SOUND         // Compile sounds.

#ifdef INPUT_GYRO
#include <Wire.h>
#endif

#ifdef OUTPUT_MAX
#include <LedControl.h>
#endif

#ifdef OUTPUT_TFT
#include <TFT.h>
#include <SPI.h>
#endif

#ifdef OUTPUT_MAX
#define PIN_DATA          8
#define PIN_LOAD          9
#define PIN_CLOCK         10

LedControl matrix = LedControl(PIN_DATA, PIN_CLOCK, PIN_LOAD, 5);
#endif // OUTPUT_MAX

#ifdef OUTPUT_TFT
#define PIN_CS            10
#define PIN_DC            9
#define PIN_RESET         8

TFT tft = TFT(PIN_CS, PIN_DC, PIN_RESET);
#endif // OUTPUT_TFT

#define PIN_SWITCH        2

#ifdef INPUT_KEYS
#define PIN_BUTTON_LEFT   6
#define PIN_BUTTON_RIGHT  7
#define PIN_BUTTON_DOWN   5
#define PIN_BUTTON_UP     4
#endif // INPUT_KEYS

#ifdef INPUT_GYRO
#define MPU               0x68
#define GYRO_SENSITIVITY  2500
#endif // INPUT_GYRO

#ifdef OUTPUT_MAX
#define SCREEN_WIDTH      16
#define SCREEN_HEIGHT     16
#define PLAYGROUND_WIDTH  12
#define PLAYGROUND_HEIGHT 15
#define MATRIX_SIZE       8
#endif // OUTPUT_MAX

#ifdef OUTPUT_TFT
#define SCREEN_WIDTH_IN_PIXEL   128
#define SCREEN_HEIGHT_IN_PIXEL  160
#define SCREEN_WIDTH            16
#define SCREEN_HEIGHT           20
#define PLAYGROUND_WIDTH        12
#define PLAYGROUND_HEIGHT       19
#define PLAYGROUND_LEFT         (SCREEN_WIDTH - PLAYGROUND_WIDTH) / 2
#define PLAYGROUND_RIGHT        PLAYGROUND_LEFT + PLAYGROUND_WIDTH - 1
#define PLAYGROUND_BOTTOM       1
#define PLAYGROUND_TOP          PLAYGROUND_BOTTOM + PLAYGROUND_HEIGHT - 1
#define PIXEL_SIZE              8
#endif // OUTPUT_TFT

#ifdef SOUND
#define PIN_PIEZO         3
#endif

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif

#ifdef INPUT_GYRO

  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

#endif // INPUT_GYRO

  pinMode(PIN_SWITCH, INPUT);

#ifdef INPUT_KEYS

  pinMode(PIN_BUTTON_LEFT, INPUT);
  pinMode(PIN_BUTTON_RIGHT, INPUT);
  pinMode(PIN_BUTTON_DOWN, INPUT);
  pinMode(PIN_BUTTON_UP, INPUT);

#endif // INPUT_KEYS

#ifdef OUTPUT_MAX

  for (byte matrixDevice = 0; matrixDevice < matrix.getDeviceCount(); matrixDevice++)
  {
    matrix.shutdown(matrixDevice, false);
  }

#endif // OUTPUT_MAX

#ifdef OUTPUT_TFT

  tft.begin();
  tft.background(0, 0, 0);
  tft.stroke(0x08, 0x08, 0x08);

  for (byte grid = 0; grid <= PLAYGROUND_WIDTH; grid++)
  {
    byte coordinateX = ((SCREEN_WIDTH - PLAYGROUND_WIDTH) / 2) * PIXEL_SIZE + (grid * PIXEL_SIZE);
    tft.line(0, coordinateX, SCREEN_HEIGHT_IN_PIXEL, coordinateX);
  }

  for (byte grid = 0; grid <= PLAYGROUND_HEIGHT; grid++)
  {
    byte startX = ((SCREEN_WIDTH - PLAYGROUND_WIDTH) / 2) * PIXEL_SIZE;
    byte endX = (SCREEN_WIDTH - ((SCREEN_WIDTH - PLAYGROUND_WIDTH) / 2)) * PIXEL_SIZE;
    byte coordinateY = ((SCREEN_HEIGHT - PLAYGROUND_HEIGHT) / 2) * PIXEL_SIZE + (grid * PIXEL_SIZE);
    tft.line(SCREEN_HEIGHT_IN_PIXEL - coordinateY, startX, SCREEN_HEIGHT_IN_PIXEL - coordinateY, endX);
  }

#endif // OUTPUT_TFT

#ifdef SOUND
  pinMode(PIN_PIEZO, OUTPUT);
#endif

  startNewGame(false);
}

void loop()
{
  handleControls();

  if (moveFigureDown() == false)
    startNewGame(true);
}

#define NUMBER_OF_FIGURES 7
#define FIGURE_SIZE       4

const byte figures[NUMBER_OF_FIGURES][4][FIGURE_SIZE][2] =
{
  {
    { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  2,  0 } },
    { {  0, -1 }, {  0,  0 }, {  0,  1 }, {  0,  2 } },
    { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  2,  0 } },
    { {  0, -1 }, {  0,  0 }, {  0,  1 }, {  0,  2 } }
  },
  {
    { { -1, -1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } },
    { { -1,  1 }, {  0,  1 }, {  0,  0 }, {  0, -1 } },
    { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  1,  1 } },
    { {  0,  1 }, {  0,  0 }, {  0, -1 }, {  1, -1 } }
  },
  {
    { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  1, -1 } },
    { { -1, -1 }, {  0, -1 }, {  0,  0 }, {  0,  1 } },
    { { -1,  1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } },
    { {  0, -1 }, {  0,  0 }, {  0,  1 }, {  1,  1 } }
  },
  {
    { { -1,  0 }, {  0,  0 }, {  0, -1 }, {  1, -1 } },
    { { -1, -1 }, { -1,  0 }, {  0,  0 }, {  0,  1 } },
    { { -1,  0 }, {  0,  0 }, {  0, -1 }, {  1, -1 } },
    { { -1, -1 }, { -1,  0 }, {  0,  0 }, {  0,  1 } }
  },
  {
    { { -1, -1 }, {  0, -1 }, {  0,  0 }, {  1,  0 } },
    { {  0,  1 }, {  0,  0 }, {  1,  0 }, {  1, -1 } },
    { { -1, -1 }, {  0, -1 }, {  0,  0 }, {  1,  0 } },
    { {  0,  1 }, {  0,  0 }, {  1,  0 }, {  1, -1 } }
  },
  {
    { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  0, -1 } },
    { {  0,  1 }, {  0,  0 }, {  0, -1 }, { -1,  0 } },
    { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  0,  1 } },
    { {  0,  1 }, {  0,  0 }, {  0, -1 }, {  1,  0 } }
  },
  {
    { {  0,  0 }, {  1,  0 }, {  1, -1 }, {  0, -1 } },
    { {  0,  0 }, {  1,  0 }, {  1, -1 }, {  0, -1 } },
    { {  0,  0 }, {  1,  0 }, {  1, -1 }, {  0, -1 } },
    { {  0,  0 }, {  1,  0 }, {  1, -1 }, {  0, -1 } }
  }
};

byte screen[SCREEN_WIDTH][SCREEN_HEIGHT];

struct Figure
{
  byte x;
  byte y;
  byte type;
  byte color;
  byte rotation;
  bool matrix[FIGURE_SIZE][FIGURE_SIZE];
};

struct Figure figure;

bool figureIsDropingDown;

#ifdef INPUT_GYRO

struct Vector
{
  int16_t x, y, z;
};

struct Vector accelerometerInit;
struct Vector gyroscopeInit;

#endif // INPUT_GYRO

void handleControls()
{
  if (digitalRead(PIN_SWITCH) == LOW)
  {

#ifdef INPUT_KEYS

    if (figureIsDropingDown == false)
    {
      bool pressedLeft = false;
      bool pressedRight = false;
      bool pressedDown = false;
      bool pressedUp = false;

      for (unsigned int iteration = 0; iteration < 15000; iteration++)
      {
        if (digitalRead(PIN_BUTTON_LEFT) == HIGH)
        {
          pressedLeft = true;
        }
        if (digitalRead(PIN_BUTTON_RIGHT) == HIGH)
        {
          pressedRight = true;
        }
        if (digitalRead(PIN_BUTTON_DOWN) == HIGH)
        {
          pressedDown = true;
        }
        if (digitalRead(PIN_BUTTON_UP) == HIGH)
        {
          pressedUp = true;
        }
      }

      if (pressedLeft == true)
        moveFigureLeft();

      if (pressedRight == true)
        moveFigureRight();

      if (pressedDown == true)
        dropFigure();

      if (pressedUp == true)
        turnFigure();
    }
    else
    {
      delay(50);
    }

#endif // INPUT_KEYS

  }
  else
  {

#ifdef INPUT_GYRO

    if (figureIsDropingDown == false)
    {
      struct Vector accelerometer;
      struct Vector gyroscope;

      Wire.beginTransmission(MPU);
      Wire.write(0x3B);
      Wire.endTransmission(false);
      Wire.requestFrom(MPU, 12, true);

      accelerometer.x = Wire.read() << 8 | Wire.read();
      accelerometer.y = Wire.read() << 8 | Wire.read();
      accelerometer.z = Wire.read() << 8 | Wire.read();

      gyroscope.x = Wire.read() << 8 | Wire.read();
      gyroscope.y = Wire.read() << 8 | Wire.read();
      gyroscope.z = Wire.read() << 8 | Wire.read();

#ifdef DEBUG
      Serial.print("Accelerometer (X/Y/Z): ");
      Serial.print(accelerometer.x);
      Serial.print(" / ");
      Serial.print(accelerometer.y);
      Serial.print(" / ");
      Serial.println(accelerometer.z);

      Serial.print("Gyroscope (X/Y/Z): ");
      Serial.print(gyroscope.x);
      Serial.print(" / ");
      Serial.print(gyroscope.y);
      Serial.print(" / ");
      Serial.println(gyroscope.z);
#endif // DEBUG

      if (accelerometerInit.x - accelerometer.x > +GYRO_SENSITIVITY)
      {
        moveFigureLeft();
      }
      else if (accelerometerInit.x - accelerometer.x < -GYRO_SENSITIVITY)
      {
        moveFigureRight();
      }
      else if (accelerometerInit.z - accelerometer.z < -GYRO_SENSITIVITY)
      {
        dropFigure();
      }
      else if (accelerometerInit.z - accelerometer.z > +GYRO_SENSITIVITY)
      {
        turnFigure();
      }

      delay(400);
    }
    else
    {
      delay(50);
    }

#endif // INPUT_GYRO

  }
}

void resetPlayground()
{
  figureIsDropingDown = false;

  for (byte y = 0; y < SCREEN_HEIGHT; y++)
  {
    for (byte x = 0; x < SCREEN_WIDTH; x++)
    {
      setPointState(x, y, B00110000);
    }
  }

  for (byte y = 0; y < PLAYGROUND_HEIGHT; y++)
  {
    for (byte x = 0; x < PLAYGROUND_WIDTH; x++)
    {
      setPointState(((SCREEN_WIDTH - PLAYGROUND_WIDTH) / 2) + x, 1 + y, 0);
    }
  }
}

void startNewGame(bool withWait)
{
  if (digitalRead(PIN_SWITCH) == LOW)
  {
#ifdef INPUT_KEYS

    if (withWait == true)
    {
      for (;;)
      {
        if (digitalRead(PIN_BUTTON_LEFT) == HIGH)
          break;
        else if (digitalRead(PIN_BUTTON_RIGHT) == HIGH)
          break;
        else if (digitalRead(PIN_BUTTON_DOWN) == HIGH)
          break;
        else if (digitalRead(PIN_BUTTON_UP) == HIGH)
          break;
      }
    }

#endif // INPUT_KEYS
  }
  else
  {
#ifdef INPUT_GYRO

    delay(500);

    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 12, true);

    accelerometerInit.x = Wire.read() << 8 | Wire.read();
    accelerometerInit.y = Wire.read() << 8 | Wire.read();
    accelerometerInit.z = Wire.read() << 8 | Wire.read();

    gyroscopeInit.x = Wire.read() << 8 | Wire.read();
    gyroscopeInit.y = Wire.read() << 8 | Wire.read();
    gyroscopeInit.z = Wire.read() << 8 | Wire.read();

#endif // INPUT_GYRO
  }

  resetPlayground();
  createNewFigure();
}

bool createNewFigure()
{
  figureIsDropingDown = false;

  figure.x = (SCREEN_WIDTH / 2) - (FIGURE_SIZE / 2);
  figure.y = SCREEN_HEIGHT - (FIGURE_SIZE / 2);
  figure.type = random(0, NUMBER_OF_FIGURES);
  figure.color = random(0, 255);
  figure.rotation = 0;

  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      figure.matrix[x][y] = false;
    }
  }

  for (byte pixel = 0; pixel < FIGURE_SIZE; pixel++)
  {
    byte offsetX = 1 + figures[figure.type][figure.rotation][pixel][0];
    byte offsetY = 1 + figures[figure.type][figure.rotation][pixel][1];
    figure.matrix[offsetX][offsetY] = true;
  }

  if (figureFitsOnPlayground() == true)
  {
    showFigure();
    return true;
  }
  else
  {
    return false;
  }
}

void hideFigure()
{
  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      if (figure.matrix[x][y] == true)
        setPointState(figure.x + x, figure.y + y, 0);
    }
  }
}

void showFigure()
{
  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      if (figure.matrix[x][y] == true)
        setPointState(figure.x + x, figure.y + y, figure.color);
    }
  }
}

bool figureFitsOnPlayground()
{
  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      if ((figure.matrix[x][y] == true) && (getPointState(figure.x + x, figure.y + y) != 0))
        return false;
    }
  }

  return true;
}

void moveFigureLeft()
{
  bool canMove = true;

  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      if (figure.matrix[x][y] == true)
      {
        byte screenX = figure.x + x;
        byte screenY = figure.y + y;

        if (getPointState(screenX - 1, screenY) != 0)
        {
          canMove = false;
          break;
        }

        break;
      }
    }

    if (canMove == false)
      break;
  }

  if (canMove == true)
  {
    hideFigure();
    figure.x--;
    showFigure();
  }
}

void moveFigureRight()
{
  bool canMove = true;

  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      if (figure.matrix[FIGURE_SIZE - 1 - x][y] == true)
      {
        byte screenX = figure.x + FIGURE_SIZE - 1 - x;
        byte screenY = figure.y + y;

        if (getPointState(screenX + 1, screenY) != 0)
        {
          canMove = false;
          break;
        }

        break;
      }
    }

    if (canMove == false)
      break;
  }

  if (canMove == true)
  {
    hideFigure();
    figure.x++;
    showFigure();
  }
}

void turnFigure()
{
  hideFigure();

  figure.rotation = (figure.rotation == 0) ? 3 : figure.rotation - 1;

  for (byte y = 0; y < FIGURE_SIZE; y++)
  {
    for (byte x = 0; x < FIGURE_SIZE; x++)
    {
      figure.matrix[x][y] = false;
    }
  }

  for (byte pixel = 0; pixel < FIGURE_SIZE; pixel++)
  {
    byte offsetX = 1 + figures[figure.type][figure.rotation][pixel][0];
    byte offsetY = 1 + figures[figure.type][figure.rotation][pixel][1];

    if (getPointState(figure.x + offsetX, figure.y + offsetY) != 0)
    {
      figure.rotation = (figure.rotation == 0) ? 3 : figure.rotation - 1;
      break;
    }
  }

  for (byte pixel = 0; pixel < FIGURE_SIZE; pixel++)
  {
    byte offsetX = 1 + figures[figure.type][figure.rotation][pixel][0];
    byte offsetY = 1 + figures[figure.type][figure.rotation][pixel][1];
    figure.matrix[offsetX][offsetY] = true;
  }

  showFigure();
}

void dropFigure()
{
  figureIsDropingDown = true;
}

bool moveFigureDown()
{
  bool canMove = true;

  for (byte x = 0; x < FIGURE_SIZE; x++)
  {
    for (byte y = 0; y < FIGURE_SIZE; y++)
    {
      if (figure.matrix[x][y] == true)
      {
        byte screenX = figure.x + x;
        byte screenY = figure.y + y;

        if (getPointState(screenX, screenY - 1) != 0)
        {
          canMove = false;
          break;
        }

        break;
      }
    }

    if (canMove == false)
      break;
  }

  if (canMove == true)
  {
    hideFigure();
    figure.y--;
    showFigure();
  }
  else
  {
    delay(200);

#ifdef SOUND
    tone(PIN_PIEZO, 500);
    delay(20);
    tone(PIN_PIEZO, 1000, 40);
#endif

    removeFullLines();

    bool newFigureCreated = createNewFigure();
    if (newFigureCreated == true)
    {
      showFigure();
    }
    else
    {
      delay(200);
      return false;
    }

    delay(100);
  }

  return true;
}

void removeFullLines()
{
  for (byte y = PLAYGROUND_BOTTOM; y <= PLAYGROUND_TOP; y++)
  {
    for (;;)
    {
      bool lineIsFull = true;

      for (byte x = PLAYGROUND_LEFT; x <= PLAYGROUND_RIGHT; x++)
      {
        if (getPointState(x, y) == 0)
        {
          lineIsFull = false;
        }
      }

      if (lineIsFull == false)
        break;

      for (byte x = PLAYGROUND_LEFT; x <= PLAYGROUND_RIGHT; x++)
        setPointState(x, y, 0);

      delay(500);

      for (byte y2 = y; y2 <= PLAYGROUND_TOP; y2++)
      {
        for (byte x = PLAYGROUND_LEFT; x <= PLAYGROUND_RIGHT; x++)
        {
          setPointState(x, y2, getPointState(x, y2 + 1));
          setPointState(x, y2 + 1, 0);
        }
      }

#ifdef SOUND
      tone(PIN_PIEZO, 1000);
      delay(20);
      tone(PIN_PIEZO, 1600);
      delay(10);
      tone(PIN_PIEZO, 1800);
      delay(20);
      tone(PIN_PIEZO, 1600);
      delay(10);
      tone(PIN_PIEZO, 1200, 40);
#endif
    }
  }
}

byte getPointState(byte x, byte y)
{
  if ((x < SCREEN_HEIGHT) && (y < SCREEN_HEIGHT))
    return screen[x][y];
  else
    return false;
}

void setPointState(byte x, byte y, byte state)
{
  if (y < SCREEN_HEIGHT)
  {
    if (screen[x][y] != state)
    {
      screen[x][y] = state;

#ifdef OUTPUT_MAX

      byte deviceNumber, rowNumber, columnNumber;

      if (x < MATRIX_SIZE)
      {
        if (y < MATRIX_SIZE)
        {
          deviceNumber = 1;
          rowNumber = x;
          columnNumber = y;
        }
        else
        {
          deviceNumber = 0;
          rowNumber = x;
          columnNumber = y - MATRIX_SIZE;
        }
      }
      else
      {
        if (y < MATRIX_SIZE)
        {
          deviceNumber = 3;
          rowNumber = x - MATRIX_SIZE;
          columnNumber = y;
        }
        else
        {
          deviceNumber = 2;
          rowNumber = x - MATRIX_SIZE;
          columnNumber = y - MATRIX_SIZE;
        }
      }

      matrix.setLed(deviceNumber + 1, rowNumber, columnNumber, (state == 0) ? false : true);

#endif // OUTPUT_MAX

#ifdef OUTPUT_TFT

      tft.stroke(
          ((state >> 4) & B11) * 80,
          ((state >> 2) & B11) * 80,
          ((state)      & B11) * 80);
      byte coordinateX = x * PIXEL_SIZE + (PIXEL_SIZE - 1);
      byte coordinateY = y * PIXEL_SIZE + (PIXEL_SIZE - 1);
      tft.rect(
          SCREEN_HEIGHT_IN_PIXEL - coordinateY,
          SCREEN_WIDTH_IN_PIXEL - coordinateX,
          PIXEL_SIZE - 1,
          PIXEL_SIZE - 1);
      tft.rect(
          SCREEN_HEIGHT_IN_PIXEL - coordinateY + 1,
          SCREEN_WIDTH_IN_PIXEL - coordinateX + 1,
          PIXEL_SIZE - 3,
          PIXEL_SIZE - 3);
    }

#endif // OUTPUT_TFT
  }
}
