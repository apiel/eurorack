// This is buggy and block usb upload. Instead of having the sequencer, trigger it manually.

#include <Arduino.h>
#include <DigiUSB.h>

const int PIN_SNARE = 0;   // Pin with the LED (P0 or P1)
const int PIN_CLOCK = 1;
const int PIN_SHIFT = 2;   
const int PIN_ANALOG_BUTTONS = 3; 

// Timing & Logic
int bpm = 160;
unsigned long lastStepTime = 0;
int stepCounter = 0;

// Debouncing and scrolling for BPM buttons
unsigned long lastBpmChange = 0;
const int scrollSpeed = 500; 

void setup() {
  pinMode(PIN_SNARE, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT, INPUT_PULLUP);
  randomSeed(analogRead(1)); 
  delay(2000);
}

// --- New Function to Generate Snare Noise ---
void triggerSnare() {
  // A snare is basically 30-50ms of noise
  // We loop and toggle the pin randomly to create "white noise"
  for (int i = 0; i < 600; i++) {
    // Generate random 1 or 0
    digitalWrite(PIN_SNARE, random(0, 2));
    
    // As 'i' increases, we add a tiny delay to create a "decay" effect
    // This makes the noise sound "lo-fi" and then fade out
    if (i > 400) delayMicroseconds(i / 10); 
  }
  digitalWrite(PIN_SNARE, LOW); // Ensure pin is off
}

void loop() {
  DigiUSB.refresh(); // if using USB

  // --- Read Analog Buttons on P3 ---
  int analogVal = analogRead(3); 
  bool repeatPressed = (analogVal > 900);             
  bool mutePressed   = (analogVal > 500 && analogVal < 900); 
  bool shiftHeld     = (digitalRead(PIN_SHIFT) == LOW);

  // --- BPM Change Logic (Only when Shift is held) ---
  if (shiftHeld) {
    if (mutePressed || repeatPressed) {
      if (millis() - lastBpmChange >= scrollSpeed) {
        bpm = mutePressed ? min(bpm + 5, 300) : max(bpm - 5, 40);
        lastBpmChange = millis();
      }
    } else {
      lastBpmChange = 0;
    }
  }

  // --- Sequencer Logic ---
  unsigned long stepDuration = 15000 / bpm;

  if (millis() - lastStepTime >= stepDuration) {
    lastStepTime = millis();

    bool isRepeat = (repeatPressed && !shiftHeld);
    bool isMuted  = (mutePressed && !shiftHeld);
    bool triggerNow = false;

    int positionInBar = stepCounter % 4;
    int loopPos = stepCounter % 32;

    if (isRepeat) {
      triggerNow = (positionInBar == 0 || positionInBar == 2);
    } else {
      if (positionInBar == 0) triggerNow = true;
      else if (loopPos > 28 && random(100) < 70) triggerNow = true;
    }

    // --- Output Execution ---
    digitalWrite(PIN_CLOCK, HIGH); // Still send clock to Edge
    
    if (triggerNow && !isMuted) {
      triggerSnare(); // Call our noise generator instead of a 10ms pulse
    }

    delay(5); // Short pulse for the Clock
    digitalWrite(PIN_CLOCK, LOW);

    stepCounter++;
  }
}