/* Wiring:
- Roll servo - pin 9
- Pitch servo - pin 10
- Red RGB pin - pin 6
- Green RGB pin - pin 7
- Blue RGB pin - pin 8
- Battery sensor - pin 0
- SDA - pin 4
- SCL - pin 5
*/ 

#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Servo.h>
#include <PID_v1.h>

Adafruit_ICM20948 icm;
Servo rollServo;  
Servo pitchServo; 

// Angle Calculation Variables
float rollAngle = 0, pitchAngle = 0;
unsigned long lastTime;

// PID Variables
double SetpointRoll = 0, InputRoll, OutputRoll;
double SetpointPitch = 0, InputPitch, OutputPitch;
double Kp = 2.2, Ki = 0.0, Kd = 0.35; 

PID pidRoll(&InputRoll, &OutputRoll, &SetpointRoll, Kp, Ki, Kd, REVERSE);
PID pidPitch(&InputPitch, &OutputPitch, &SetpointPitch, Kp, Ki, Kd, REVERSE);

// Pin Definitions
const int batteryPin = 0; 
const int redPin     = 6; 
const int greenPin   = 7; 
const int bluePin    = 8; 

void setLED(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void checkBattery() {
  int sensorValue = analogRead(batteryPin);
  float voltage = sensorValue * (3.3 / 1023.0) * 2.0; 
  if (voltage > 3.8) setLED(0, 0, 255);       
  else if (voltage > 3.5) setLED(255, 150, 0); 
  else setLED(255, 0, 0);                      
}

void setup() {
  Wire.begin();
  Wire.setClock(400000);
  
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setLED(255, 255, 255);

  rollServo.attach(9);   
  pitchServo.attach(10);  

  rollServo.write(90);
  pitchServo.write(90);

  if (!icm.begin_I2C()) {
    while (1) { 
      setLED(255, 0, 0); delay(100); 
      setLED(0,0,0); delay(100); 
    }
  }

  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
  icm.setGyroRange(ICM20948_GYRO_RANGE_500_DPS);

  delay(2000);

  pidRoll.SetMode(AUTOMATIC);
  pidRoll.SetOutputLimits(-60, 60);
  pidRoll.SetSampleTime(10);

  pidPitch.SetMode(AUTOMATIC);
  pidPitch.SetOutputLimits(-60, 60);
  pidPitch.SetSampleTime(10);

  lastTime = millis();
  checkBattery();
}

void loop() {
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;

  icm.getEvent(&accel, &gyro, &temp, &mag);

  float dt = (millis() - lastTime) / 1000.0;
  lastTime = millis();

  float rollAcc = atan2(accel.acceleration.y, accel.acceleration.z) * 57.2958;
  float pitchAcc = atan2(-accel.acceleration.x,
                        sqrt(accel.acceleration.y * accel.acceleration.y +
                             accel.acceleration.z * accel.acceleration.z)) * 57.2958;

  rollAngle = 0.98 * (rollAngle + gyro.gyro.x * dt * 57.2958) + 0.02 * rollAcc;
  pitchAngle = 0.98 * (pitchAngle + gyro.gyro.y * dt * 57.2958) + 0.02 * pitchAcc;

  InputRoll = rollAngle;
  InputPitch = pitchAngle;

  // Keep level (no adaptive drift)
  SetpointRoll = 0;
  SetpointPitch = 0;

  pidRoll.Compute();
  pidPitch.Compute();

  // Roll normal
  rollServo.write(90 + OutputRoll);

  // 🔥 Pitch reversed (THIS is the key change)
  pitchServo.write(90 - OutputPitch);

  static unsigned long lastBat = 0;
  if (millis() - lastBat > 5000) {
    checkBattery();
    lastBat = millis();
  }
}