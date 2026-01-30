#pragma once

#include <cmath>
#include <cstdint>

// Set this to 1 to test hardware with a simple beep, 0 for the real Kick
#define DEBUG_BEEP 0

class Kick2Engine {
public:
    float baseFrequency = 45.0f;
    float sweepDepth = 600.0f;
    float compressionAmount = 0.5f;
    float driveAmount = 0.6f;

    float phase = 0.0f;
    float pitchEnv = 1.0f;
    float clickEnv = 1.0f;

    void trigger() {
        phase = 0.0f;
        pitchEnv = 1.0f;
        clickEnv = 1.0f;
    }

    uint16_t process(float velocity) {
        if (pitchEnv < 0.0001f) return 2048; // Return silence if envelope died
        
        #if DEBUG_BEEP
            // Simple 440Hz-ish Square Wave for hardware testing
            phase += 0.01f; 
            if (phase > 1.0f) phase -= 1.0f;
            return (phase > 0.5f) ? 3000 : 1000; 
        #else
            // 1. Envelopes
            pitchEnv *= 0.9994f; 
            clickEnv *= 0.992f;

            // 2. Oscillator
            float currentFreq = baseFrequency + (sweepDepth * pitchEnv);
            // Increased increment to ensure it's audible in manual loops
            phase += currentFreq / 15000.0f; 
            if (phase > 1.0f) phase -= 1.0f;
            
            float rawSine = std::sin(2.0f * 3.14159265f * phase);
            
            // 3. Waveshaper
            float shaped = (rawSine + driveAmount * (rawSine * rawSine * rawSine)) / (1.0f + driveAmount);

            // 4. Final Mix (Centered at 0.0)
            float finalMix = shaped * pitchEnv * velocity;

            // 5. Map to DAC (Centered at 2048)
            // We apply compression here on the 0.0 to 1.0 range
            float normalized = (finalMix + 1.0f) * 0.5f;
            
            // Simple Soft Compression
            normalized = std::pow(normalized, 1.0f - compressionAmount * 0.5f);
            
            int32_t out = static_cast<int32_t>(normalized * 4095.0f);

            if (out > 4095) out = 4095;
            if (out < 0) out = 0;
            
            return static_cast<uint16_t>(out);
        #endif
    }
};

static Kick2Engine kick;