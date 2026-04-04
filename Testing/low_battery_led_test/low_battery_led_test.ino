const int batteryPin = A0;
const int ledPin = 13;

const float referenceVoltage = 5.09;
const float thresholdVoltage = 3.50;
const float dividerRatio = 2.0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int rawValue = analogRead(batteryPin);
  float measuredBatteryVoltage = (rawValue * referenceVoltage / 1024.0) * dividerRatio;

  Serial.print(measuredBatteryVoltage);
  Serial.println("V");

  if (measuredBatteryVoltage < thresholdVoltage) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

  delay(1000);
}