#include <Servo.h>

Servo testServo;

#define SERVO_PIN 9
#define CENTER_ANGLE 90
#define REFRESH_INTERVAL 5000

struct TestStep {
  float frequency;
  float amplitude;
  unsigned long duration;
};

TestStep schedule[] = {
  {0.5, 25, 3000},
  {0.0, 0,  2000},
  {1.0, 25, 3000},
  {0.0, 0,  2000},
  {2.0, 30, 3000},
  {0.0, 0,  2000},
  {4.0, 30, 3000},
  {0.0, 0,  2000},
  {6.0, 25, 3000},
  {0.0, 0,  2000},
  {8.0, 25, 3000}
};

int currentStep = 0;
int totalSteps = sizeof(schedule) / sizeof(schedule[0]);
unsigned long stepStartTime;

void setup() {
  Serial.begin(115200);
  testServo.attach(SERVO_PIN);
  stepStartTime = millis();
}

void loop() {
  unsigned long now = millis();
  unsigned long elapsed = now - stepStartTime;

  if (elapsed >= schedule[currentStep].duration) {
    currentStep++;
    stepStartTime = now;
    elapsed = 0;
    
    if (currentStep >= totalSteps) {
      currentStep = 0; 
    }
  }

  float freq = schedule[currentStep].frequency;
  float amp = schedule[currentStep].amplitude;
  float angle;

  if (freq <= 0.01) {
    angle = CENTER_ANGLE;
  } else {
    float t = elapsed / 1000.0;
    angle = CENTER_ANGLE + amp * sin(2 * PI * freq * t);
  }

  angle = constrain(angle, 0, 180);
  testServo.write(angle);

  static unsigned long lastPrint = 0;
  if (now - lastPrint > REFRESH_INTERVAL) {
    Serial.print(currentStep);
    Serial.print(",");
    Serial.print(freq);
    Serial.print(",");
    Serial.println(angle);
    lastPrint = now;
  }

  delay(1);
}