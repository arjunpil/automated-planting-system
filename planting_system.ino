#include <Wire.h>
#include <Servo.h>

// SA8870C I2C address
#define SA8870C_ADDR 0x34

#define ADC_BAT_ADDR                  0x00
#define MOTOR_TYPE_ADDR               0x14
#define MOTOR_ENCODER_POLARITY_ADDR   0x15
#define MOTOR_FIXED_SPEED_ADDR        0x33

#define MOTOR_TYPE_WITHOUT_ENCODER        0
#define MOTOR_TYPE_TT                     1
#define MOTOR_TYPE_N20                    2
#define MOTOR_TYPE_JGB37_520_12V_110RPM   3

Servo myServo;

// Servo control
int centerAngle = 90;
int offset = 25;

// Servo flags
bool servoActive = false;
bool servoReturning = false;
unsigned long servoStartTime = 0;

// Motor arrays
int8_t car_forward[4]   = {0, 0, -40, -40};
int8_t car_backword[4]  = {0, 0, 40, 40};
int8_t car_turnLeft[4]  = {0, 0, 0, -40};
int8_t car_turnRight[4] = {0, 0, -40, 0};
int8_t car_stop[4]      = {0, 0, 0, 0};

uint8_t MotorType = MOTOR_TYPE_JGB37_520_12V_110RPM;
uint8_t MotorEncoderPolarity = 0;

char command;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();

  Serial.begin(9600);
  Serial1.begin(9600);

  Serial.println("Bluetooth Tank Ready");

  myServo.attach(9);
  myServo.write(centerAngle);
  delay(500);
  myServo.detach();  // detach after initial position set

  WireWriteDataArray(MOTOR_TYPE_ADDR, &MotorType, 1);
  delay(5);
  WireWriteDataArray(MOTOR_ENCODER_POLARITY_ADDR, &MotorEncoderPolarity, 1);
}

void loop() {

  // Stage 1: move back counterclockwise
  if (servoActive && !servoReturning && millis() - servoStartTime >= 600) {
    while(Serial1.available()) Serial1.read();
    Serial.println("Returning to center");
    myServo.write(centerAngle + offset);  // move back counterclockwise
    servoReturning = true;
    servoStartTime = millis();  // reset timer for stage 2
  }

  // Stage 2: done, shut off servo
  if (servoActive && servoReturning && millis() - servoStartTime >= 600) {
    while(Serial1.available()) Serial1.read();
    Serial.println("Servo done");
    myServo.detach();  // shut off servo signal
    servoActive = false;
    servoReturning = false;
  }

  if (Serial1.available()) {

    command = Serial1.read();

    Serial.print("Received: ");
    Serial.println(command);

    if (command == 'S' && !servoActive) {
      Serial.println("Square button pressed");
      servoActive = true;
      servoReturning = false;
      servoStartTime = millis();
      myServo.attach(9);  // re-attach each time
      while(Serial1.available()) Serial1.read();
      Serial.println("Moving to offset");
      myServo.write(centerAngle - offset);  // clockwise
    }

    if (!servoActive) {
      switch(command) {

        case 'F':
          WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_forward, 4);
        break;

        case 'B':
          WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_backword, 4);
        break;

        case 'L':
          WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_turnLeft, 4);
        break;

        case 'R':
          WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_turnRight, 4);
        break;

        case 'A':
          Serial.println("Start command");
        break;

        case 'P':
          Serial.println("Pause command");
        break;

        case '0':
          WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_stop, 4);
        break;
      }
    }
  }
}

// I2C motor driver function
bool WireWriteDataArray(uint8_t reg, uint8_t *val, unsigned int len)
{
    Wire.beginTransmission(SA8870C_ADDR);
    Wire.write(reg);

    for(unsigned int i = 0; i < len; i++) {
        Wire.write(val[i]);
    }

    return (Wire.endTransmission() == 0);
}
