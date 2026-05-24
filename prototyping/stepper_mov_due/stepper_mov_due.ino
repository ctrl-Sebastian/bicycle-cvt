#include <AccelStepper.h>

// ESP32 Motor Pins (The ones we proved work!)
#define DIR_PIN   14 
#define STEP_PIN  25

// ESP32 Analog Pin for Joystick
#define JOY_Y_PIN 34

// Initialize AccelStepper for a stepper driver (1 = DRV8825)
AccelStepper stepper(1, STEP_PIN, DIR_PIN);

// Set your maximum safe shifting speed here
float maxSpeed = 1500.0; 

void setup() {
  Serial.begin(115200);

  // Configure Stepper Limits
  stepper.setMaxSpeed(maxSpeed);
  
  Serial.println("ESP32 Analog Joystick Control Ready.");
}

void loop() {
  // 1. Read the Joystick's current Y-axis position (0 to 4095 on ESP32)
  int joyVal = analogRead(JOY_Y_PIN);
  
  // 2. Define the ESP32 Deadband limits (Center is roughly 2048)
  int deadbandLow = 1800;
  int deadbandHigh = 2300;
  
  // 3. Translate joystick position to motor speed
  if (joyVal > deadbandHigh) {
    // Stick is pushed UP
    // Map the upper half of the stick (2300 to 4095) to positive speed
    float currentSpeed = map(joyVal, deadbandHigh, 4095, 0, maxSpeed);
    stepper.setSpeed(currentSpeed);
    stepper.runSpeed();
  } 
  else if (joyVal < deadbandLow) {
    // Stick is pulled DOWN
    // Map the lower half of the stick (1800 to 0) to negative speed
    float currentSpeed = map(joyVal, deadbandLow, 0, 0, -maxSpeed);
    stepper.setSpeed(currentSpeed);
    stepper.runSpeed();
  } 
  else {
    // Stick is resting in the Deadband center
    // Do nothing. The hardwired EN pin to Ground will maintain holding torque.
  }
}