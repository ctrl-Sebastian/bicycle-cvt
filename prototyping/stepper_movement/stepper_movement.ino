#include <AccelStepper.h>

// --- NOTICE THE NEW DIR_PIN! ---
#define DIR_PIN   14  // Moved away from the potentially dead Pin 26
#define STEP_PIN  25
#define EN_PIN    27

// Initialize AccelStepper for a stepper driver (1 = DRV8825)
AccelStepper stepper(1, STEP_PIN, DIR_PIN);

void setup() {
  // Enable the driver (Active LOW)
  pinMode(EN_PIN, OUTPUT);
  digitalWrite(EN_PIN, LOW);

  // Configure Stepper Limits for High Inertia
  stepper.setMaxSpeed(1000.0);      
  stepper.setAcceleration(500.0);   
}

void loop() {
  // 1. Move DOWN (Negative direction)
  // 2000 steps = 10 full revolutions (20mm of travel on an 8x2 lead screw)
  stepper.moveTo(-2000);
  stepper.runToPosition(); // Blocks everything until the move is done
  
  delay(1000); // Rest for 1 second
  
  // 2. Move UP (Positive direction back to starting point)
  stepper.moveTo(0);
  stepper.runToPosition(); 
  
  delay(1000); // Rest for 1 second
}