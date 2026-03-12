#include <Adafruit_ICM20X.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_ICM20948 icm;

// Variables for Filter
float roll = 0, pitch = 0, yaw = 0;
float gyroX_bias = 0, gyroY_bias = 0, gyroZ_bias = 0;
unsigned long last_time;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);

  if (!icm.begin_I2C()) {
    Serial.println("Failed to find ICM20948");
    while (1) delay(10);
  }

  // --- CALIBRATION ---
  // We MUST calculate the Gyro "bias" (error) while the sensor is still
  Serial.println("Calibrating Gyro... Do not move sensor!");
  int samples = 200;
  for (int i = 0; i < samples; i++) {
    sensors_event_t accel, gyro, temp, mag;
    icm.getEvent(&accel, &gyro, &temp, &mag);
    gyroX_bias += gyro.gyro.x;
    gyroY_bias += gyro.gyro.y;
    gyroZ_bias += gyro.gyro.z;
    delay(5);
  }
  gyroX_bias /= samples;
  gyroY_bias /= samples;
  gyroZ_bias /= samples;

  last_time = millis();
  Serial.println("Calibration Complete. Starting at 0,0,0");
}

void loop() {
  sensors_event_t accel, gyro, temp, mag;
  icm.getEvent(&accel, &gyro, &temp, &mag);

  // Calculate time delta (dt)
  unsigned long currentTime = millis();
  float dt = (currentTime - last_time) / 1000.0;
  last_time = currentTime;

  // 1. Subtract Bias from Gyro (convert radians to degrees)
  float gx = (gyro.gyro.x - gyroX_bias) * 57.296;
  float gy = (gyro.gyro.y - gyroY_bias) * 57.296;
  float gz = (gyro.gyro.z - gyroZ_bias) * 57.296;

  // 2. Calculate Pitch & Roll from Accelerometer
  float rollAcc = atan2(accel.acceleration.y, accel.acceleration.z) * 57.296;
  float pitchAcc = atan2(-accel.acceleration.x, sqrt(accel.acceleration.y * accel.acceleration.y + accel.acceleration.z * accel.acceleration.z)) * 57.296;

  // 3. Apply Complementary Filter
  // Roll and Pitch use Accel to "fix" Gyro drift
  roll = 0.96 * (roll + gx * dt) + 0.04 * rollAcc;
  pitch = 0.96 * (pitch + gy * dt) + 0.04 * pitchAcc;
  
  // Yaw has no Accel reference, so it is "Pure Integration" 
  // It will start at 0 but may drift slightly over long periods
  yaw += gz * dt;

  // Output
  Serial.print(pitch);
  Serial.print(",");
  Serial.print(roll);
  Serial.print(",");
  Serial.println(yaw); // Use println for the last value to signal a new line

  delay(10); // Higher sample rate is better for integration
}