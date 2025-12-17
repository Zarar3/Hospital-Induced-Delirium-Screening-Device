# Hospital-Induced-Delirium-Screening-Device
Created a prototype for a theoretical screening device hospitals can use to screen patients for delirium.
# Quick Start Guide 


## Quick Setup 

### 1. Voice Interface Setup

```bash
# Install Python libraries
pip install pyserial pyttsx3 speechrecognition pyaudio

# Find your Arduino port (Windows example)
# Device Manager → Ports → Note COM port (e.g., COM3)

# Run the voice interface
python voice_interface.py --port COM3
```

**First run will:**
- Initialize text-to-speech
- Calibrate microphone (stay quiet for 2 seconds)
- Connect to Arduino
- Say "System ready"

### 2. Using Voice Interface

**Press START on Arduino or type 's' in terminal**

The laptop will:
- Read questions aloud
- Listen for "true" or "false"
- Provide audio feedback
- Announce assessment results

## File Overview

| File | Purpose |
|------|---------|
| `voice_interface.py` | Voice control script |
| `delirium_diagnostic_EXPANDED.ino` | Arduino code (30 questions) |
| `COMPLETE_SETUP_GUIDE.md` | Full documentation |

---

## Troubleshooting

### Voice not working?
```bash
# Test microphone first
python -c "import speech_recognition as sr; print(sr.Microphone.list_microphone_names())"

# Adjust volume in code:
# Edit voice_interface.py line ~43
self.tts_engine.setProperty('volume', 1.0)  # Max volume
```

### Arduino not connecting?
```bash
# List available ports:
python -m serial.tools.list_ports
```

---

## What It Does

### Cognitive Test:
- 8 random questions from bank of 30
- Voice reads question: "5 plus 3 equals 8?"
- Patient presses Button A
- Voice confirms: "Correct!"
- LCD shows colored feedback (orange→yellow/red)

### Reaction Test:
- Voice says "Press Button A"
- LCD shows ">>> A <<<"
- Patient presses as fast as possible
- Voice gives feedback "Good reaction!"

### Results:
- Voice announces: "Assessment complete. Low risk detected."
- LCD shows risk assessment
- Serial output shows detailed scores


Group 22: Zarar Khan, Arnav Shah, Justin Fang, Jasper Wong
