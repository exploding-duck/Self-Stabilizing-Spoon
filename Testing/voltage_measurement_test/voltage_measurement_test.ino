const int analogPin = A0;
float vIn = 0.0;
float vOut = 0.0;

float referenceVoltage = 5.09; 

void setup() {
  Serial.begin(115200);
  Serial.println("Voltmeter Initialized");
}

void loop() {
  int rawValue = analogRead(analogPin);

  vOut = (rawValue * referenceVoltage) / 1023.0;

  vIn = vOut * 2.0;

  Serial.print("Voltage: ");

  Serial.print(vIn);
  Serial.println("V");

  delay(500); 
}
