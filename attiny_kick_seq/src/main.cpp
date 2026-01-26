#include <Arduino.h>

const int PIN_TRIGGER = 0; // with Led
const int PIN_CLOCK = 1;
const int PIN_SHIFT = 2;   // Hold this to change BPM
// P3 will now handle both Repeat and Mute via analogRead
const int PIN_ANALOG_BUTTONS = 3; 

// Timing & Logic
int bpm = 160;
unsigned long lastStepTime = 0;
int stepCounter = 0;

// Debouncing and scrolling for BPM buttons
unsigned long lastBpmChange = 0;
const int scrollSpeed = 250; // Time in ms between BPM changes when held

void setup()
{
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT, INPUT_PULLUP);
  // P3 doesn't need INPUT_PULLUP for analogRead on Digispark (hardware pull-up exists)

  randomSeed(analogRead(1)); // Seed from P2/A1
}

void loop()
{
  // --- Read Analog Buttons on P3 ---
  int analogVal = analogRead(3); 
  
  // Thresholds based on 1.5k internal pull-up + 4.7k resistor ladder
  bool repeatPressed = (analogVal > 900);             // To 5v
  bool mutePressed   = (analogVal > 500 && analogVal < 900); // to 3.3v or Through 4.7k
  
  bool shiftHeld = (digitalRead(PIN_SHIFT) == LOW);

  // --- BPM Change Logic (Only when Shift is held) ---
  if (shiftHeld)
  {
    if (mutePressed || repeatPressed)
    {
      // Use a timer to debounce and allow "scrolling" when held
      if (millis() - lastBpmChange >= scrollSpeed)
      {
        bpm = mutePressed ? min(bpm + 5, 300) : max(bpm - 5, 40);
        lastBpmChange = millis();
      }
    }
  }

  // --- Sequencer Logic ---
  // Note: 15000 / bpm = 16th notes. If too fast, change to 30000 (8th notes).
  unsigned long stepDuration = 15000 / bpm;

  if (millis() - lastStepTime >= stepDuration)
  {
    lastStepTime = millis();

    bool isRepeat = (repeatPressed && !shiftHeld);
    bool isMuted  = (mutePressed && !shiftHeld);
    bool triggerNow = false;

    int positionInBar = stepCounter % 4;
    int loopPos = stepCounter % 32;

    if (isRepeat)
    {
      triggerNow = (positionInBar == 0 || positionInBar == 2);
    }
    else
    {
      if (positionInBar == 0)
      {
        triggerNow = true;
      }
      else if (loopPos > 28 && random(100) < 70)
      {
        triggerNow = true;
      }
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
