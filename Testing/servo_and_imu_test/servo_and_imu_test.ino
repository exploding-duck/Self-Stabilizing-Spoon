#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Servo.h>

#define REFRESH_INTERVAL 5000

Adafruit_ICM20948 icm;
Servo rollServo;

float roll = 0;
float gyroX_bias = 0;
unsigned long last_time;

void setup(void) {
  Serial.begin(115200);
  rollServo.attach(9);
  rollServo.write(90); 

  if (!icm.begin_I2C()) {
    Serial.println("Failed to find ICM20948");
    while (1) delay(10);
  }
  
  Serial.println("Calibrating");
  int samples = 100;

  for (int i = 0; i < samples; i++) {
    sensors_event_t accel, gyro, temp, mag;
    icm.getEvent(&accel, &gyro, &temp, &mag);
    gyroX_bias += gyro.gyro.x;
    delay(5);
  }

  gyroX_bias /= samples;
  last_time = millis();
}

void loop() {
  sensors_event_t accel, gyro, temp, mag;
  icm.getEvent(&accel, &gyro, &temp, &mag);

  unsigned long currentTime = millis();
  float dt = (currentTime - last_time) / 1000.0;
  last_time = currentTime;

  float gx = (gyro.gyro.x - gyroX_bias) * 57.296;
  float rollAcc = atan2(accel.acceleration.y, accel.acceleration.z) * 57.296;
  
  roll = 0.96 * (roll + gx * dt) + 0.04 * rollAcc;

  int servoPos = map(roll, -90, 90, 0, 180);
  
  servoPos = constrain(servoPos, 0, 180);

  rollServo.write(180-servoPos);

  Serial.print("Roll: "); Serial.print(roll);
  Serial.print(" -> Servo: "); Serial.println(servoPos);

  delay(15);
}