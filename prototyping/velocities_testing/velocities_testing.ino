#include <Servo.h>

// --- Hardware Pins ---
const int hallSensorPin = 2; // Receives the pulse from the sensor
const int servoPin = 3;      // Controls the physical metronome

// Create the Servo object
Servo magnetServo;

// --- Sensor Variables (Shared with Interrupt) ---
// 'volatile' tells the compiler these can change at any millisecond
volatile unsigned long lastPulseTime = 0;
volatile unsigned long deltaTime = 0;
volatile bool newPulse = false;

// --- Metronome Timing Variables ---
unsigned long lastSweepTime = 0;
const unsigned long sweepInterval = 1000; // 1000ms = 1 full sweep cycle per second
bool magnetIsNear = false;

// --- Bicycle Constants ---
// Standard 29-inch (700c) wheel circumference is ~2.07 meters
const float wheelCircumference = 2.07; 

// The Interrupt Service Routine (ISR) for the DUE
void hallSensorISR() {
  unsigned long currentTime = millis();
  
  // Software Debounce (Ignore rapid phantom triggers under 20ms)
  if (currentTime - lastPulseTime > 20) { 
    deltaTime = currentTime - lastPulseTime;
    lastPulseTime = currentTime;
    newPulse = true;
  }
}

void setup() {
  // The DUE has two USB ports. 
  // 'Serial' talks to the "Programming" port closest to the power jack.
  Serial.begin(115200);

  // 1. Setup the Servo
  magnetServo.attach(servoPin);
  
  // 2. Setup the Hall Sensor
  pinMode(hallSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(hallSensorPin), hallSensorISR, FALLING);

  Serial.println("Arduino DUE CVT Test Rig Initialized.");
  Serial.println("Sweeping magnet once per second...");
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. The Non-Blocking Servo Metronome ---
  // Every 1000ms, sweep the arm over the sensor
  if (currentMillis - lastSweepTime >= sweepInterval) {
    lastSweepTime = currentMillis;
    magnetServo.write(90); // Position where magnet triggers sensor
    magnetIsNear = true;
  }
  
  // 150ms later, pull the arm back away from the sensor
  if (magnetIsNear && (currentMillis - lastSweepTime >= 150)) {
    magnetServo.write(45); // Position away from sensor
    magnetIsNear = false;
  }

  // --- 2. Process the Sensor Data & Math ---
  if (newPulse) {
    // Briefly pause interrupts to safely copy the data
    noInterrupts();
    unsigned long dt = deltaTime;
    newPulse = false;
    interrupts();

    // Prevent division by zero
    if (dt > 0) {
      // RPM Math: 60,000ms in a minute / (dt * 3 magnets)
      // Simplified: 20000.0 / dt
      float rpm = 20000.0 / dt; 
      
      // Linear Speed Math: (RPM / 60) * Circumference = meters per second
      float speed_ms = (rpm / 60.0) * wheelCircumference;
      
      // Convert to Kilometers per Hour (multiply by 3.6)
      float speed_kmh = speed_ms * 3.6;

      // Print Results
      Serial.print("Pulse dt: ");
      Serial.print(dt);
      Serial.print("ms | RPM: ");
      Serial.print(rpm);
      Serial.print(" | Speed: ");
      Serial.print(speed_ms);
      Serial.print(" m/s (");
      Serial.print(speed_kmh);
      Serial.println(" km/h)");
    }
  }

  // --- 3. Timeout Logic (Wheel Stopped) ---
  if (currentMillis - lastPulseTime > 2000) {
    static unsigned long lastZeroPrint = 0;
    // Print the zero state every 500ms so we don't flood the serial monitor
    if (currentMillis - lastZeroPrint > 500) {
      Serial.println("Status: 0.00 RPM | 0.00 km/h (Wheel Stopped)");
      lastZeroPrint = currentMillis;
    }
  }
}