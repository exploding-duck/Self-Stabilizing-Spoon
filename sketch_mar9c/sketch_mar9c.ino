/*
 * Simple Voltmeter with 1:2 Voltage Divider (two 10k resistors)
 */

const int analogPin = A0;
float vIn = 0.0;
float vOut = 0.0;

// Calibration: Measure your Arduino's 5V pin with a multimeter 
// and put the exact value here (e.g., 4.98 or 5.02) for better accuracy.
float referenceVoltage = 5.09; 

void setup() {
  Serial.begin(115200);
  Serial.println("Voltmeter Initialized");
}

void loop() {
  // 1. Read the raw ADC value (0 to 1023)
  int rawValue = analogRead(analogPin);

  // 2. Convert ADC to voltage seen at the pin (0V to 5V)
  vOut = (rawValue * referenceVoltage) / 1023.0;

  // 3. Double the value to account for the 10k/10k divider
  vIn = vOut * 2.0;

  // 4. Print results

  Serial.print("Voltage: ");

  Serial.print(vIn);
  Serial.println("V");

  delay(500); // Half-second updates
}
