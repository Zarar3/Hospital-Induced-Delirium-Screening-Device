import serial # To use the serial number given by the Arduino
import time # Used to track time
import win32com.client # To use Winodws text-to-speech software

# Simple voice interface using Windows SAPI directly
speaker = win32com.client.Dispatch("SAPI.SpVoice")
speaker.Rate = 3  # Speed up speech (default is 0, range is -10 to 10)

# Set voice
voices = speaker.GetVoices()
speaker.Voice = voices.Item(1)

# connecting to the Arduino
print("Connecting to Arduino on COM3...")
arduino = serial.Serial('COM3', 9600, timeout=1) #Displays the connection between the python script and the Arduino
time.sleep(2)
print("Connected!")

# Speaker introduces user to the system
speaker.Speak("System ready. Starting test.") 
speaker.Speak("Choose button A if you are below 40, button B if you are 40 or above.")
speaker.Speak("Press any button to start")

# Error cases for the TTS to ignore, like when the arduino is loading between lines
while True:
    if arduino.in_waiting > 0:
        line = arduino.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        print(f"Arduino: {line}")# Writes what the Arduino gives to the python script

        # Speak actual test questions (Q1/8: format)
        if "?" in line and "/" in line and line.startswith("Q"):
            question = line.split(":", 1)[1].strip()
            print(f"Speaking: {question}") # Speaks the question

            #Special cases for the questions with the minus sign so the program can read it properly
            if ("7 - 3 = 4" in question): 
                speaker.Speak("Seven minus three equals four")
            elif ("10 - 4 = 5" in question):
                speaker.Speak("Ten minus four equals five")
            else:
                speaker.Speak(question)

        # Speak reaction test prompts
        elif "Press Button A" in line:
            speaker.Speak("A")
        elif "Press Button B" in line:
            speaker.Speak("B")
        elif "MISSED" in line:
            speaker.Speak("Missed")

        # Speak feedback
        elif "CORRECT!" in line:
            speaker.Speak("Correct")
        elif "TIMEOUT" in line:
            speaker.Speak("Time out")
        elif "INCORRECT" in line:
            speaker.Speak("Incorrect")
        elif "REACTION TEST STARTING" in line:
            speaker.Speak("Starting reaction test.")
        elif "LOW RISK" in line:
            speaker.Speak("Low risk detected.")
        elif "MODERATE RISK" in line:
            speaker.Speak("Moderate risk detected.")
        elif "HIGH RISK" in line:
            speaker.Speak("High risk detected.")

    time.sleep(0.01) # Small delay so the speaker doesn't rush too much
