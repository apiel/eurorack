#include <Arduino.h>

const int PIN_TRIGGER = 0; // with Led
const int PIN_CLOCK = 1;
const int PIN_SHIFT = 2;  // Hold this to change BPM
const int PIN_REPEAT = 3; // Also BPM UP
const int PIN_MUTE = 4;   // Also BPM DOWN


// Timing & Logic
int bpm = 160;
unsigned long lastStepTime = 0;
int stepCounter = 0;

// Debouncing for BPM buttons
bool upPressed = false;
bool downPressed = false;

void setup()
{
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT, INPUT_PULLUP);
  pinMode(PIN_REPEAT, INPUT_PULLUP);
  pinMode(PIN_MUTE, INPUT_PULLUP);

  randomSeed(analogRead(A1)); // Seed from the now-empty Analog pin
}

void loop()
{
  bool shiftHeld = (digitalRead(PIN_SHIFT) == LOW);

  // --- BPM Change Logic (Only when Shift is held) ---
  if (shiftHeld)
  {
    // Check UP button
    if (digitalRead(PIN_REPEAT) == LOW && !upPressed)
    {
      bpm = min(bpm + 5, 300); // Cap at 300 BPM
      upPressed = true;
    }
    else if (digitalRead(PIN_REPEAT) == HIGH)
    {
      upPressed = false;
    }

    // Check DOWN button
    if (digitalRead(PIN_MUTE) == LOW && !downPressed)
    {
      bpm = max(bpm - 5, 40); // Floor at 40 BPM
      downPressed = true;
    }
    else if (digitalRead(PIN_MUTE) == HIGH)
    {
      downPressed = false;
    }
  }

  // --- Sequencer Logic ---
  unsigned long stepDuration = 15000 / bpm;

  if (millis() - lastStepTime >= stepDuration)
  {
    lastStepTime = millis();

    // bool isMuted = (digitalRead(PIN_MUTE) == LOW && !shiftHeld);
    bool isRepeat = (digitalRead(PIN_REPEAT) == LOW && !shiftHeld);
    bool isMuted = false;
    // bool isRepeat = false;
    bool triggerNow = false;

    int positionInBar = stepCounter % 4;
    int loop = stepCounter % 32;

    if (isRepeat)
    {
      triggerNow = positionInBar == 0 || positionInBar == 2;
    }
    else
    {
      if (positionInBar == 0)
      {
        triggerNow = true;
      }
      // last 4 steps of a bar
      else if (loop > 28 && random(100) < 70)
      {
        triggerNow = true;
      }
      // else if (random(100) < 1)
      // {
      //   triggerNow = true;
      // }
    }

    // --- Output Execution ---
    digitalWrite(PIN_CLOCK, HIGH);
    if (triggerNow && !isMuted)
    {
      digitalWrite(PIN_TRIGGER, HIGH);
    }

    delay(10);
    digitalWrite(PIN_CLOCK, LOW);
    digitalWrite(PIN_TRIGGER, LOW);

    stepCounter++;
  }
}