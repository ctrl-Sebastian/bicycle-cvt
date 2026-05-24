int hallEffectSensorPin = 4;

volatile unsigned long lastPulseTime = 0;
volatile unsigned long deltaTime = 0;
volatile bool newPulse = false;

// The Interrupt Service Routine (ISR)
// IRAM_ATTR tells the ESP32 to keep this function in RAM for maximum speed
void IRAM_ATTR hallSensorISR() {
  unsigned long currentTime = millis();
  
  // Software Debounce: Ignore pulses that happen impossibly fast (e.g., < 20ms)
  // 20ms between pulses with 3 magnets equals 1000 RPM, which a bicycle won't hit!
  if (currentTime - lastPulseTime > 20) {
    deltaTime = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
    newPulse = true;
  }
}

void setup() {
  Serial.begin(115200);

  // Set the pin mode. Use INPUT_PULLUP if your sensor doesn't have a built-in pull-up resistor.
  pinMode(hallEffectSensorPin, INPUT_PULLUP);

  // Attach the interrupt. FALLING triggers when the magnet approaches (signal goes from HIGH to LOW)
  attachInterrupt(digitalPinToInterrupt(hallEffectSensorPin), hallSensorISR, FALLING);
}

void loop() {
  // 1. Check if a new magnet passed by
  if (newPulse) {
    // Briefly pause interrupts to safely copy the volatile variables
    noInterrupts();
    unsigned long dt = deltaTime;
    newPulse = false;
    interrupts();

    // Prevent division by zero just in case
    if (dt > 0) {
      // Calculate RPM using our simplified formula
      float rpm = 20000.0 / dt;

      Serial.print("Delta Time: ");
      Serial.print(dt);
      Serial.print(" ms | RPM: ");
      Serial.println(rpm);
    }
  }

  // 2. Timeout Logic: What if the bicycle stops?
  // If no magnet has passed in the last 2000ms (2 seconds), consider the wheel stopped.
  if (millis() - lastPulseTime > 2000) {
    // Only print "0" occasionally so we don't spam the Serial Monitor
    static unsigned long lastZeroPrint = 0;
    if (millis() - lastZeroPrint > 500) {
      Serial.println("RPM: 0.00 (Stopped)");
      lastZeroPrint = millis();
    }
  }

}
