/*
Date: 4/1/2026
Author: Matteo Coscia
Functionality: This code is used to control a stabilizing spoon that helps cancel tremors while maintaining intentional movements. It uses an adaptive setpoint to control the setpoint and to control the motors. It will cancel frequencies above ~4 Hz while letting frequencies below it through.
*/

// Importing all libraries
#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Servo.h>

// Defining all variables
#define ROLL_PIN 9   
#define PITCH_PIN 10   

Adafruit_ICM20948 icm;
Servo rollServo; 
Servo pitchServo;  

float roll = 0;   
float pitch = 0;  
float rollSetpoint = 0; 
float pitchSetpoint = 0; 

// Alpha controls the filter. Lower - more cancelling, but stiffer. Higher - smoother, but less cancelling
const float alpha = 0.03; 

float gyroX_bias = 0; 
float gyroY_bias = 0; 
unsigned long last_time;

void setup(void) {
  // Set up serial
  Serial.begin(115200);
  
  rollServo.attach(ROLL_PIN);
  pitchServo.attach(PITCH_PIN);
  
  // Move servos to neutral
  rollServo.write(90); 
  pitchServo.write(90); 

  if (!icm.begin_I2C()) {
    while (1) delay(10);
  }
  
  // Set I2C communication rate
  Wire.setClock(400000); 

  // Calibrate
  int samples = 500; 
  for (int i = 0; i < samples; i++) {
    sensors_event_t accel, gyro, temp, mag;
    icm.getEvent(&accel, &gyro, &temp, &mag);
    gyroX_bias += gyro.gyro.x;
    gyroY_bias += gyro.gyro.y; 
    delay(2);
  }

  gyroX_bias /= (float)samples;
  gyroY_bias /= (float)samples;
  last_time = micros(); // Precision with micros
}

void loop() {
  sensors_event_t accel, gyro, temp, mag;
  icm.getEvent(&accel, &gyro, &temp, &mag);

  unsigned long currentTime = micros();
  float dt = (currentTime - last_time) / 1000000.0;
  
  // Check interval rate
  if (dt < 0.004) return; 
  last_time = currentTime;

  // Calculate orientation
  float gx = (gyro.gyro.x - gyroX_bias) * 57.296;
  float gy = (gyro.gyro.y - gyroY_bias) * 57.296;

  float rollAcc = atan2(accel.acceleration.y, accel.acceleration.z) * 57.296;
  float pitchAcc = atan2(-accel.acceleration.x, sqrt(accel.acceleration.y * accel.acceleration.y + accel.acceleration.z * accel.acceleration.z)) * 57.296;

  // Increase gyro trust of accelerometer
  roll = 0.99 * (roll + gx * dt) + 0.01 * rollAcc;
  pitch = 0.99 * (pitch + gy * dt) + 0.01 * pitchAcc;

  // Setpoints
  rollSetpoint = ((1.0 - alpha) * rollSetpoint) + (alpha * roll);
  pitchSetpoint = ((1.0 - alpha) * pitchSetpoint) + (alpha * pitch);

  // Roll servo
  float rollError = roll - rollSetpoint;
  int rollPos = map(rollError, -90, 90, 0, 180);
  rollPos = constrain(rollPos, 0, 180);
  rollServo.write(rollPos); 

  // Pitch servo
  float pitchError = pitch - pitchSetpoint;

  // Angle compensation
  float rollRad = roll * 0.0174533; 
  float cosRoll = cos(rollRad);
  // Dont divide by zero
  if (abs(cosRoll) < 0.1) cosRoll = 0.1;
  float correctedPitchError = pitchError / cosRoll; 

  int pitchPos = map(correctedPitchError, -90, 90, 0, 180); 
  pitchPos = constrain(pitchPos, 0, 180);
  
  pitchServo.write(pitchPos); 
}