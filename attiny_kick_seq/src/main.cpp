#include <Arduino.h>

// TODO try to use to move trigger to p4
// Add input on P0 maybe for bpm
// then use P2 as analog input for potentiometer

const int PIN_TBD = 0;
const int PIN_CLOCK = 1; // with Led
const int PIN_SHIFT = 2; // Hold this to change BPM
const int PIN_ANALOG_BUTTONS = 3;
const int PIN_TRIGGER = 4;

// Timing & Logic
int bpm = 160;
unsigned long lastStepTime = 0;
int stepCounter = 0;

// Debouncing and scrolling for BPM buttons
unsigned long lastBpmChange = 0;
const int scrollSpeed = 150; // Time in ms between BPM changes when held

void setup()
{
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_CLOCK, OUTPUT);
  pinMode(PIN_SHIFT, INPUT_PULLUP);
  pinMode(PIN_TBD, INPUT_PULLUP);
  // P3 doesn't need INPUT_PULLUP for analogRead on Digispark (hardware pull-up exists)

  randomSeed(analogRead(1)); // Seed from P2/A1
}

void loop()
{
  // DigiUSB.refresh(); // if using USB, need #include <DigiUSB.h>

  // --- Read Analog Buttons on P3 ---
  int analogVal = analogRead(PIN_ANALOG_BUTTONS);

  // Thresholds based on 1.5k internal pull-up + 4.7k resistor ladder
  bool repeatPressed = (analogVal > 900);                  // To 5v
  bool mutePressed = (analogVal > 500 && analogVal < 900); // to 3.3v or Through 4.7k

  bool shiftHeld = (digitalRead(PIN_SHIFT) == LOW);
  bool tbdPressed = (digitalRead(PIN_TBD) == LOW);

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
    bool isMuted = (mutePressed && !shiftHeld);
    bool triggerNow = false;

    int positionInBar = stepCounter % 4;
    int loopPos64 = stepCounter % 64;
    int loopPos32 = stepCounter % 32;

    if (tbdPressed)
    {
      // Instead of this, we could TBD to set different patterns...
      // Now it random between step 28 and 32
      // instead we could set either 28/32 or 60/64
      triggerNow = true;
    }
    else if (isRepeat)
    {
      triggerNow = (positionInBar == 0 || positionInBar == 2);
    }
    else if (positionInBar == 0)
    {
      triggerNow = true;
    }
    else if (loopPos64 > 60 && random(100) < 60)
    {
      triggerNow = true;
    }
    // Let's make those step less probable
    else if ((loopPos32 == 29 || loopPos32 == 31) && random(100) < 20)
    {
      triggerNow = true;
    }
    // in the end only step 30 is at 40% but we keep this logic in case we remove the previous condition
    else if (loopPos32 > 28 && random(100) < 40)
    {
      triggerNow = true;
    }
    else
    {
      triggerNow = random(100) < 1;
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
