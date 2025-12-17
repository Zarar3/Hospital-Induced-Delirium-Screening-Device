/*
 * Hospital Induced Delirium Diagnostic Device
 * COMPLETE VERSION - All Fixes and Improvements Included
 * 
 * FEATURES:
 * ✓ Compilation error fixed
 * ✓ Smooth color transitions (gentle on patients)
 * ✓ Randomized question bank (15 questions, asks 8)
 * ✓ Softer, calming colors
 * ✓ Patient-friendly interface
 * 
 * Hardware:
 * - Arduino UNO R4 Minima
 * - Grove Base Shield
 * - Grove LCD RGB Backlight (I2C)
 * - 2x Tactile Buttons (4-pin, on breadboard)
 * 
 * Wiring:
 * - LCD: I2C port on Grove Base Shield
 * - Button A: Pin 2 → GND (across breadboard gap)
 * - Button B: Pin 3 → GND (across breadboard gap)
 * 
 * Group 22: Zarar Khan, Arnav Shah, Justin Fang, Jasper Wong
 */

#include <Wire.h>
#include "rgb_lcd.h"

rgb_lcd lcd;

// ============================================
// PIN DEFINITIONS
// ============================================
const int BUTTON_A_PIN = 2;  // Left button (True/A)
const int BUTTON_B_PIN = 3;  // Right button (False/B)
const int STATUS_LED = 13;   // Built-in LED

// ============================================
// LCD COLORS - Orange/Yellow/Red Theme
// ============================================
const int COLOR_READY[3] = {255, 140, 0};       // Orange - Ready/Neutral state
const int COLOR_QUESTION[3] = {255, 140, 0};    // Orange - Questions (neutral)
const int COLOR_CORRECT[3] = {255, 200, 0};     // Yellow - Correct answer
const int COLOR_WRONG[3] = {255, 50, 0};        // Red - Wrong answer
const int COLOR_COMPLETE[3] = {255, 180, 0};    // Yellow-orange - Complete
const int COLOR_NEUTRAL[3] = {255, 140, 0};     // Orange - Neutral

// ============================================
// TEST CONFIGURATION
// ============================================
const int TOTAL_QUESTIONS_IN_BANK = 15;  // Total questions available
const int NUM_QUESTIONS_TO_ASK = 8;      // How many to ask per test
const int NUM_REACTION_TESTS = 10;       // Number of reaction tests
const int REACTION_TIMEOUT = 5000;       // 5 seconds max response time

// ============================================
// COLOR TRANSITION SETTINGS (Adjust for desired speed)
// ============================================
// Current settings: 20 steps × 5ms = 100ms (very fast, visible)
// For slower: 40 steps × 8ms = 320ms (more dramatic)
// For instant: 1 step × 0ms = instant (not recommended)
const int COLOR_TRANSITION_STEPS = 20;   // Fewer steps = faster transition
const int COLOR_TRANSITION_DELAY = 5;    // Delay per step in milliseconds

// ============================================
// RESULT DISPLAY TIMING
// ============================================
const int COGNITIVE_RESULT_DISPLAY = 2500;  // How long to show correct/wrong (ms)
const int REACTION_RESULT_DISPLAY = 1500;   // How long to show reaction result (ms)

// ============================================
// TEST STATES
// ============================================
enum TestState {
  IDLE,
  COGNITIVE_TEST,
  REACTION_TEST,
  RESULTS,
  EMERGENCY_STOP
};

TestState currentState = IDLE;

// ============================================
// FORWARD DECLARATIONS (Fixes compilation error)
// ============================================
void displayFinalResults();
void setLCDColorSmooth(int targetR, int targetG, int targetB);

// ============================================
// DATA STORAGE
// ============================================
struct TestResults {
  // Cognitive Test
  int cognitiveCorrect = 0;
  int cognitiveTotal = 0;
  unsigned long cognitiveResponseTimes[15];
  int cognitiveAttempts = 0;
  
  // Reaction Test
  int reactionCorrect = 0;
  int reactionTotal = 0;
  unsigned long reactionTimes[20];
  int reactionAttempts = 0;
  int missedReactions = 0;
  int wrongButtonPresses = 0;
};

TestResults results;

// Current LCD color (for smooth transitions)
int currentR = 0, currentG = 0, currentB = 0;

// ============================================
// QUESTION BANK - 15 Questions (Randomized)
// ============================================
struct Question {
  String text;
  bool correctAnswer;  // true = Button A, false = Button B
};

Question questionBank[TOTAL_QUESTIONS_IN_BANK] = {
  // Math questions (5)
  {"5 + 3 = 8?", true},
  {"10 - 4 = 5?", false},
  {"2 + 2 = 5?", false},
  {"7 - 3 = 4?", true},
  {"6 + 2 = 9?", false},
  
  // Time and dates (4)
  {"Week has 10 days?", false},
  {"Week has 7 days?", true},
  {"Month has 40 days?", false},
  {"Year has 12 months?", true},
  
  // General knowledge (6)
  {"Sky is blue?", true},
  {"Hand has 5 fingers?", true},
  {"Sun rises in east?", true},
  {"Water is wet?", true},
  {"Fire is cold?", false},
  {"Grass is green?", true}
};

// Array to track which questions have been asked
bool questionAsked[TOTAL_QUESTIONS_IN_BANK];
int selectedQuestions[15];  // Indices of questions to ask this round

// ============================================
// TEST VARIABLES
// ============================================
int currentQuestion = 0;
int currentReactionTest = 0;
unsigned long testStartTime = 0;
bool waitingForResponse = false;
bool correctButton = true;
bool resultsDisplayed = false;  // Flag to prevent multiple displays

// ============================================
// SETUP
// ============================================
void setup() {
  Serial.begin(9600);
  
  // Initialize random seed (important for question randomization)
  randomSeed(analogRead(A0));
  
  // Initialize LCD
  lcd.begin(16, 2);
  currentR = COLOR_READY[0];
  currentG = COLOR_READY[1];
  currentB = COLOR_READY[2];
  lcd.setRGB(currentR, currentG, currentB);
  
  // Initialize button pins with internal pull-up resistors
  pinMode(BUTTON_A_PIN, INPUT_PULLUP);
  pinMode(BUTTON_B_PIN, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);
  
  // Status LED on indicates system ready
  digitalWrite(STATUS_LED, HIGH);
  
  // Welcome message
  Serial.println("=================================");
  Serial.println("Hospital Delirium Diagnostic Test");
  Serial.println("Group 22 - Complete Version");
  Serial.println("=================================");
  Serial.println("Features:");
  Serial.println("- Smooth color transitions");
  Serial.println("- Randomized questions");
  Serial.println("- Patient-friendly interface");
  Serial.println("=================================\n");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Delirium Test");
  lcd.setCursor(0, 1);
  lcd.print("Press to Start");
  
  resetResults();
}

// ============================================
// MAIN LOOP
// ============================================
void loop() {
  switch (currentState) {
    case IDLE:
      handleIdle();
      break;
      
    case COGNITIVE_TEST:
      handleCognitiveTest();
      break;
      
    case REACTION_TEST:
      handleReactionTest();
      break;
      
    case RESULTS:
      displayFinalResults();
      break;
      
    case EMERGENCY_STOP:
      // System halted
      break;
  }
  
  delay(10);  // Small delay for stability
}

// ============================================
// STATE HANDLERS
// ============================================

void handleIdle() {
  // Wait for any button press to start
  if (digitalRead(BUTTON_A_PIN) == LOW || digitalRead(BUTTON_B_PIN) == LOW) {
    delay(300);  // Debounce
    startCognitiveTest();
  }
}

void startCognitiveTest() {
  resultsDisplayed = false;  // Reset flag for new test
  
  Serial.println("\n=== COGNITIVE TEST STARTING ===");
  Serial.println("Button A = TRUE, Button B = FALSE");
  Serial.println("Questions randomized from bank\n");
  
  // Select random questions for this test
  selectRandomQuestions();
  
  setLCDColorSmooth(COLOR_QUESTION[0], COLOR_QUESTION[1], COLOR_QUESTION[2]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cognitive Test");
  lcd.setCursor(0, 1);
  lcd.print("A=True B=False");
  delay(2000);
  
  currentState = COGNITIVE_TEST;
  currentQuestion = 0;
  displayCognitiveQuestion();
}

void selectRandomQuestions() {
  // Reset tracking array
  for (int i = 0; i < TOTAL_QUESTIONS_IN_BANK; i++) {
    questionAsked[i] = false;
  }
  
  // Select random unique questions
  Serial.println("Selected questions for this test:");
  for (int i = 0; i < NUM_QUESTIONS_TO_ASK; i++) {
    int randomIndex;
    do {
      randomIndex = random(0, TOTAL_QUESTIONS_IN_BANK);
    } while (questionAsked[randomIndex]);
    
    selectedQuestions[i] = randomIndex;
    questionAsked[randomIndex] = true;
    
    Serial.print("  Q");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(questionBank[randomIndex].text);
  }
  Serial.println();
}

void displayCognitiveQuestion() {
  if (currentQuestion >= NUM_QUESTIONS_TO_ASK) {
    finishCognitiveTest();
    return;
  }
  
  setLCDColorSmooth(COLOR_QUESTION[0], COLOR_QUESTION[1], COLOR_QUESTION[2]);
  lcd.clear();
  
  int questionIndex = selectedQuestions[currentQuestion];
  String questionText = questionBank[questionIndex].text;
  
  // Display question on LCD
  if (questionText.length() <= 16) {
    lcd.setCursor(0, 0);
    lcd.print(questionText);
    lcd.setCursor(0, 1);
    lcd.print("Q" + String(currentQuestion + 1) + "/" + String(NUM_QUESTIONS_TO_ASK) + " A=T B=F");
  } else {
    // Split long text across two lines
    lcd.setCursor(0, 0);
    lcd.print(questionText.substring(0, 16));
    lcd.setCursor(0, 1);
    if (questionText.length() > 16) {
      lcd.print(questionText.substring(16));
    }
  }
  
  Serial.print("Q");
  Serial.print(currentQuestion + 1);
  Serial.print("/");
  Serial.print(NUM_QUESTIONS_TO_ASK);
  Serial.print(": ");
  Serial.println(questionText);
  
  waitingForResponse = true;
  testStartTime = millis();
}

void handleCognitiveTest() {
  if (!waitingForResponse) return;
  
  // Check Button A (TRUE)
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BUTTON_A_PIN) == LOW) {
      processCognitiveAnswer(true);
      return;
    }
  }
  
  // Check Button B (FALSE)
  if (digitalRead(BUTTON_B_PIN) == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      processCognitiveAnswer(false);
      return;
    }
  }
  
  // Timeout check (30 seconds)
  if (millis() - testStartTime > 30000) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TIMEOUT");
    Serial.println("TIMEOUT - Moving to next question");
    waitingForResponse = false;
    delay(1500);
    currentQuestion++;
    displayCognitiveQuestion();
  }
}

void processCognitiveAnswer(bool buttonAPressed) {
  unsigned long responseTime = millis() - testStartTime;
  
  // Store response time
  if (results.cognitiveAttempts < 15) {
    results.cognitiveResponseTimes[results.cognitiveAttempts] = responseTime;
    results.cognitiveAttempts++;
  }
  
  // Check if correct
  int questionIndex = selectedQuestions[currentQuestion];
  bool correct = (buttonAPressed == questionBank[questionIndex].correctAnswer);
  
  if (correct) {
    results.cognitiveCorrect++;
    setLCDColorSmooth(COLOR_CORRECT[0], COLOR_CORRECT[1], COLOR_CORRECT[2]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Correct!");
    lcd.setCursor(0, 1);
    lcd.print("Time: " + String(responseTime) + "ms");
    Serial.println("✓ CORRECT! Time: " + String(responseTime) + "ms");
  } else {
    setLCDColorSmooth(COLOR_WRONG[0], COLOR_WRONG[1], COLOR_WRONG[2]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Incorrect");
    lcd.setCursor(0, 1);
    lcd.print("Answer: " + String(questionBank[questionIndex].correctAnswer ? "A" : "B"));
    Serial.println("✗ INCORRECT - Correct answer: " + String(questionBank[questionIndex].correctAnswer ? "A (TRUE)" : "B (FALSE)"));
  }
  
  results.cognitiveTotal++;
  waitingForResponse = false;
  currentQuestion++;
  
  delay(COGNITIVE_RESULT_DISPLAY);  // Show result (configurable at top of code)
  displayCognitiveQuestion();
}

void finishCognitiveTest() {
  Serial.println("\n=== COGNITIVE TEST COMPLETE ===");
  Serial.print("Score: ");
  Serial.print(results.cognitiveCorrect);
  Serial.print("/");
  Serial.print(results.cognitiveTotal);
  Serial.print(" (");
  Serial.print((results.cognitiveCorrect * 100) / results.cognitiveTotal);
  Serial.println("%)");
  
  setLCDColorSmooth(COLOR_COMPLETE[0], COLOR_COMPLETE[1], COLOR_COMPLETE[2]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cognitive Done!");
  lcd.setCursor(0, 1);
  lcd.print("Score: " + String(results.cognitiveCorrect) + "/" + String(results.cognitiveTotal));
  
  delay(3000);
  startReactionTest();
}

void startReactionTest() {
  Serial.println("\n=== REACTION TEST STARTING ===");
  Serial.println("Press the button shown on screen as quickly as possible!\n");
  
  setLCDColorSmooth(COLOR_QUESTION[0], COLOR_QUESTION[1], COLOR_QUESTION[2]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reaction Test");
  lcd.setCursor(0, 1);
  lcd.print("Get ready...");
  
  currentState = REACTION_TEST;
  currentReactionTest = 0;
  
  delay(2000);
  displayReactionTest();
}

void displayReactionTest() {
  if (currentReactionTest >= NUM_REACTION_TESTS) {
    finishReactionTest();
    return;
  }
  
  // Random delay between tests (1-2.5 seconds)
  delay(random(1000, 2500));
  
  // Choose random button
  correctButton = random(0, 2) == 0;
  
  // Orange color for reaction test (neutral)
  setLCDColorSmooth(255, 140, 0);  // Orange - neutral state
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PRESS BUTTON:");
  lcd.setCursor(0, 1);
  lcd.print(">>> " + String(correctButton ? "A" : "B") + " <<<");
  
  Serial.print("Test ");
  Serial.print(currentReactionTest + 1);
  Serial.print("/");
  Serial.print(NUM_REACTION_TESTS);
  Serial.print(" - Press Button ");
  Serial.println(correctButton ? "A" : "B");
  
  waitingForResponse = true;
  testStartTime = millis();
}

void handleReactionTest() {
  if (!waitingForResponse) return;
  
  // Check Button A
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    delay(10);  // Small debounce
    if (digitalRead(BUTTON_A_PIN) == LOW) {
      processReactionAnswer(true);
      return;
    }
  }
  
  // Check Button B
  if (digitalRead(BUTTON_B_PIN) == LOW) {
    delay(10);  // Small debounce
    if (digitalRead(BUTTON_B_PIN) == LOW) {
      processReactionAnswer(false);
      return;
    }
  }
  
  // Timeout check
  if (millis() - testStartTime > REACTION_TIMEOUT) {
    setLCDColorSmooth(COLOR_WRONG[0], COLOR_WRONG[1], COLOR_WRONG[2]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Missed");
    Serial.println("MISSED!");
    results.missedReactions++;
    waitingForResponse = false;
    results.reactionTotal++;
    currentReactionTest++;
    delay(1000);
    displayReactionTest();
  }
}

void processReactionAnswer(bool buttonAPressed) {
  unsigned long reactionTime = millis() - testStartTime;
  
  // Check if correct button was pressed
  bool correct = (buttonAPressed == correctButton);
  
  if (correct) {
    results.reactionCorrect++;
    setLCDColorSmooth(COLOR_CORRECT[0], COLOR_CORRECT[1], COLOR_CORRECT[2]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Good!");
    lcd.setCursor(0, 1);
    lcd.print("Time: " + String(reactionTime) + "ms");
    Serial.print("✓ HIT! Reaction time: ");
    Serial.print(reactionTime);
    Serial.println(" ms");
    
    // Store reaction time
    if (results.reactionAttempts < 20) {
      results.reactionTimes[results.reactionAttempts] = reactionTime;
      results.reactionAttempts++;
    }
  } else {
    setLCDColorSmooth(COLOR_WRONG[0], COLOR_WRONG[1], COLOR_WRONG[2]);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong Button");
    Serial.println("✗ WRONG BUTTON!");
    results.wrongButtonPresses++;
  }
  
  results.reactionTotal++;
  waitingForResponse = false;
  currentReactionTest++;
  
  delay(REACTION_RESULT_DISPLAY);  // Show result (configurable at top of code)
  displayReactionTest();
}

void finishReactionTest() {
  Serial.println("\n=== REACTION TEST COMPLETE ===");
  Serial.print("Score: ");
  Serial.print(results.reactionCorrect);
  Serial.print("/");
  Serial.print(results.reactionTotal);
  Serial.print(" (");
  Serial.print((results.reactionCorrect * 100) / results.reactionTotal);
  Serial.println("%)");
  
  setLCDColorSmooth(COLOR_COMPLETE[0], COLOR_COMPLETE[1], COLOR_COMPLETE[2]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Reaction Done!");
  lcd.setCursor(0, 1);
  lcd.print("Score: " + String(results.reactionCorrect) + "/" + String(results.reactionTotal));
  
  delay(3000);
  currentState = RESULTS;
  displayFinalResults();
}

void displayFinalResults() {
  // Only display results once
  if (resultsDisplayed) {
    return;
  }
  resultsDisplayed = true;
  
  Serial.println("\n=====================================");
  Serial.println("    FINAL TEST RESULTS");
  Serial.println("=====================================");
  
  // Calculate scores
  int cognitivePercent = (results.cognitiveCorrect * 100) / results.cognitiveTotal;
  int reactionPercent = (results.reactionCorrect * 100) / results.reactionTotal;
  
  // Calculate average times
  unsigned long avgCognitiveTime = 0;
  if (results.cognitiveAttempts > 0) {
    for (int i = 0; i < results.cognitiveAttempts; i++) {
      avgCognitiveTime += results.cognitiveResponseTimes[i];
    }
    avgCognitiveTime /= results.cognitiveAttempts;
  }
  
  unsigned long avgReactionTime = 0;
  if (results.reactionAttempts > 0) {
    for (int i = 0; i < results.reactionAttempts; i++) {
      avgReactionTime += results.reactionTimes[i];
    }
    avgReactionTime /= results.reactionAttempts;
  }
  
  // Print to Serial
  Serial.println("\nCOGNITIVE TEST:");
  Serial.print("  Accuracy: ");
  Serial.print(results.cognitiveCorrect);
  Serial.print("/");
  Serial.print(results.cognitiveTotal);
  Serial.print(" (");
  Serial.print(cognitivePercent);
  Serial.println("%)");
  Serial.print("  Avg Response: ");
  Serial.print(avgCognitiveTime);
  Serial.println(" ms");
  
  Serial.println("\nREACTION TEST:");
  Serial.print("  Accuracy: ");
  Serial.print(results.reactionCorrect);
  Serial.print("/");
  Serial.print(results.reactionTotal);
  Serial.print(" (");
  Serial.print(reactionPercent);
  Serial.println("%)");
  Serial.print("  Missed: ");
  Serial.println(results.missedReactions);
  Serial.print("  Wrong Button: ");
  Serial.println(results.wrongButtonPresses);
  Serial.print("  Avg Reaction: ");
  Serial.print(avgReactionTime);
  Serial.println(" ms");
  
  Serial.println("\nASSESSMENT:");
  String assessment = assessDeliriumRisk();
  Serial.println(assessment);
  
  // Display on LCD with smooth transition
  setLCDColorSmooth(COLOR_COMPLETE[0], COLOR_COMPLETE[1], COLOR_COMPLETE[2]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Final Results:");
  lcd.setCursor(0, 1);
  lcd.print("C:" + String(cognitivePercent) + "% R:" + String(reactionPercent) + "%");
  delay(3000);
  
  // Show assessment with appropriate color
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Assessment:");
  lcd.setCursor(0, 1);
  if (assessment.indexOf("HIGH") >= 0) {
    setLCDColorSmooth(COLOR_WRONG[0], COLOR_WRONG[1], COLOR_WRONG[2]);
    lcd.print("HIGH RISK");
  } else if (assessment.indexOf("MODERATE") >= 0) {
    setLCDColorSmooth(COLOR_COMPLETE[0], COLOR_COMPLETE[1], COLOR_COMPLETE[2]);
    lcd.print("MODERATE RISK");
  } else {
    setLCDColorSmooth(COLOR_CORRECT[0], COLOR_CORRECT[1], COLOR_CORRECT[2]);
    lcd.print("LOW RISK");
  }
  
  delay(5000);
  
  // Flash status LED to indicate completion
  for (int i = 0; i < 5; i++) {
    digitalWrite(STATUS_LED, LOW);
    delay(200);
    digitalWrite(STATUS_LED, HIGH);
    delay(200);
  }
  
  // Return to idle
  setLCDColorSmooth(COLOR_READY[0], COLOR_READY[1], COLOR_READY[2]);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test Complete");
  lcd.setCursor(0, 1);
  lcd.print("Press to Restart");
  
  Serial.println("\n=====================================");
  Serial.println("Test complete. Press any button to restart.");
  Serial.println("=====================================\n");
  
  currentState = IDLE;
  resetResults();
}

// ============================================
// ASSESSMENT ALGORITHM
// ============================================
String assessDeliriumRisk() {
  int riskScore = 0;
  
  // Cognitive accuracy check
  int cognitivePercent = (results.cognitiveCorrect * 100) / results.cognitiveTotal;
  if (cognitivePercent < 60) riskScore += 3;
  else if (cognitivePercent < 80) riskScore += 1;
  
  // Cognitive response time check
  if (results.cognitiveAttempts > 0) {
    unsigned long totalTime = 0;
    for (int i = 0; i < results.cognitiveAttempts; i++) {
      totalTime += results.cognitiveResponseTimes[i];
    }
    unsigned long avgTime = totalTime / results.cognitiveAttempts;
    if (avgTime > 5000) riskScore += 2;
    else if (avgTime > 3000) riskScore += 1;
  }
  
  // Reaction accuracy check
  int reactionPercent = (results.reactionCorrect * 100) / results.reactionTotal;
  if (reactionPercent < 50) riskScore += 3;
  else if (reactionPercent < 70) riskScore += 1;
  
  // Missed reactions
  if (results.missedReactions > 5) riskScore += 2;
  else if (results.missedReactions > 2) riskScore += 1;
  
  // Wrong button presses
  if (results.wrongButtonPresses > 3) riskScore += 1;
  
  // Reaction time check
  if (results.reactionAttempts > 0) {
    unsigned long totalTime = 0;
    for (int i = 0; i < results.reactionAttempts; i++) {
      totalTime += results.reactionTimes[i];
    }
    unsigned long avgTime = totalTime / results.reactionAttempts;
    if (avgTime > 1500) riskScore += 2;
    else if (avgTime > 1000) riskScore += 1;
  }
  
  String result = "  Risk Score: " + String(riskScore) + "/12 - ";
  
  if (riskScore >= 7) {
    result += "HIGH RISK - Immediate evaluation recommended";
  } else if (riskScore >= 4) {
    result += "MODERATE RISK - Monitoring recommended";
  } else {
    result += "LOW RISK - Normal cognitive function";
  }
  
  return result;
}

// ============================================
// SMOOTH COLOR TRANSITION FUNCTION
// ============================================
void setLCDColorSmooth(int targetR, int targetG, int targetB) {
  // Calculate the difference between current and target colors
  int diffR = targetR - currentR;
  int diffG = targetG - currentG;
  int diffB = targetB - currentB;
  
  // Transition in small steps for smooth gradient effect
  for (int step = 0; step <= COLOR_TRANSITION_STEPS; step++) {
    int newR = currentR + (diffR * step) / COLOR_TRANSITION_STEPS;
    int newG = currentG + (diffG * step) / COLOR_TRANSITION_STEPS;
    int newB = currentB + (diffB * step) / COLOR_TRANSITION_STEPS;
    
    lcd.setRGB(newR, newG, newB);
    delay(COLOR_TRANSITION_DELAY);  // Use configurable delay (50 steps × 10ms = 500ms total)
  }
  
  // Update current color tracking
  currentR = targetR;
  currentG = targetG;
  currentB = targetB;
}

// ============================================
// RESET FUNCTION
// ============================================
void resetResults() {
  results.cognitiveCorrect = 0;
  results.cognitiveTotal = 0;
  results.cognitiveAttempts = 0;
  results.reactionCorrect = 0;
  results.reactionTotal = 0;
  results.reactionAttempts = 0;
  results.missedReactions = 0;
  results.wrongButtonPresses = 0;
  
  for (int i = 0; i < 15; i++) {
    results.cognitiveResponseTimes[i] = 0;
  }
  for (int i = 0; i < 20; i++) {
    results.reactionTimes[i] = 0;
  }
}
