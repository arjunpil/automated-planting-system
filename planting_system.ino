#include <Wire.h>

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

// Motor speed arrays (using motors 3 & 4)
int8_t car_forward[4]   = {0, 0, -23, -23};
int8_t car_backword[4]  = {0, 0, 23, 23};
int8_t car_turnLeft[4]  = {0, 0, 0, -20};
int8_t car_turnRight[4] = {0, 0, -20, 0};
int8_t car_stop[4]      = {0, 0, 0, 0};

uint8_t MotorType = MOTOR_TYPE_JGB37_520_12V_110RPM;
uint8_t MotorEncoderPolarity = 0;

char command;

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Wire.begin();

  Serial.begin(9600);    // USB (Serial Monitor)
  Serial1.begin(9600);   // Bluetooth on pins 18 (TX1), 19 (RX1)

  Serial.println("Bluetooth Tank Ready");

  WireWriteDataArray(MOTOR_TYPE_ADDR, &MotorType, 1);
  delay(5);
  WireWriteDataArray(MOTOR_ENCODER_POLARITY_ADDR, &MotorEncoderPolarity, 1);
}

void loop() {

  if (Serial1.available()) {

    command = Serial1.read();

    Serial.print("Received: ");   // debug to computer
    Serial.println(command);

    switch(command) {

      case 'F':   // Forward
        WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_forward, 4);
      break;

      case 'B':   // Backward
        WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_backword, 4);
      break;

      case 'L':   // Left
        WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_turnLeft, 4);
      break;

      case 'R':   // Right
        WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_turnRight, 4);
      break;

      case 'S':
        Serial.println("Square button pressed");
      break;

      case 'A':
        Serial.println("Start command");
      break;

      case 'P':
        Serial.println("Pause command");
      break;

      case '0':   // Stop
        WireWriteDataArray(MOTOR_FIXED_SPEED_ADDR, car_stop, 4);
      break;
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
