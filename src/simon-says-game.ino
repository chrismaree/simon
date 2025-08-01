#include <Arduino.h>
#include <TM1637Display.h>
#include "pitches.h"

/* Define pin numbers for LEDs, buttons and speaker: */
const uint8_t buttonPins[] = {5, 4, 2, 15};    // Red, Green, Blue, Yellow buttons
const uint8_t ledPins[] = {18, 19, 22, 21};    // Red, Green, Blue, Yellow LEDs
#define SPEAKER_PIN 13                          // Buzzer

// These are connected to TM1637Display (used to show game score):
const int SCORE_CLK_PIN = 32;                   // Display CLK
const int SCORE_DIO_PIN = 33;                   // Display DIO

#define MAX_GAME_LENGTH 100

const int gameTones[] = { NOTE_G3, NOTE_C4, NOTE_E4, NOTE_G5};

/* Global variables - store the game state */
uint8_t gameSequence[MAX_GAME_LENGTH] = {0};
uint8_t gameIndex = 0;

// Initialize display
TM1637Display display(SCORE_CLK_PIN, SCORE_DIO_PIN);

/**
   Set up the Arduino board and initialize Serial communication
*/
void setup() {
  // Initialize serial first
  Serial.begin(115200);
  delay(1000);  // Give time for serial to initialize
  Serial.println("Simon Says for XIAO ESP32-C3");

  // Initialize display first
  display.clear();
  display.setBrightness(0x0f, true);
  delay(100);  // Give display time to initialize

  // Initialize pins
  for (byte i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);  // Ensure LEDs start OFF
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  pinMode(SPEAKER_PIN, OUTPUT);
  digitalWrite(SPEAKER_PIN, LOW);  // Ensure speaker starts OFF

  // Show startup animation
  uint8_t data[] = {0xff, 0xff, 0xff, 0xff};
  display.setSegments(data);
  delay(500);
  display.clear();
  delay(500);
  display.showNumberDec(0, true);
}

void displayScore() {
  // Show the score with leading zeros
  display.showNumberDec(gameIndex, true);
}

/**
   Lights the given LED and plays a suitable tone
*/
void lightLedAndPlayTone(byte ledIndex) {
  digitalWrite(ledPins[ledIndex], HIGH);
  tone(SPEAKER_PIN, gameTones[ledIndex]);
  delay(300);
  digitalWrite(ledPins[ledIndex], LOW);
  noTone(SPEAKER_PIN);
}

/**
   Plays the current sequence of notes that the user has to repeat
*/
void playSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte currentLed = gameSequence[i];
    lightLedAndPlayTone(currentLed);
    delay(50);
  }
}

/**
    Waits until the user pressed one of the buttons,
    and returns the index of that button
*/
byte readButtons() {
  while (true) {
    for (byte i = 0; i < 4; i++) {
      byte buttonPin = buttonPins[i];
      if (digitalRead(buttonPin) == LOW) {
        return i;
      }
    }
    delay(1);
  }
}

/**
  Play the game over sequence, and report the game score
*/
void gameOver() {
  Serial.print("Game over! your score: ");
  Serial.println(gameIndex - 1);
  gameIndex = 0;
  delay(200);

  // Play a Wah-Wah-Wah-Wah sound
  tone(SPEAKER_PIN, NOTE_DS5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(300);
  tone(SPEAKER_PIN, NOTE_CS5);
  delay(300);
  for (byte i = 0; i < 10; i++) {
    for (int pitch = -10; pitch <= 10; pitch++) {
      tone(SPEAKER_PIN, NOTE_C5 + pitch);
      delay(6);
    }
  }
  noTone(SPEAKER_PIN);

  // Display dashes for game over
  uint8_t segments[] = {
    SEG_G,           // -
    SEG_G,           // -
    SEG_G,           // -
    SEG_G            // -
  };
  display.setSegments(segments);
  delay(500);
}

/**
   Get the user's input and compare it with the expected sequence.
*/
bool checkUserSequence() {
  for (int i = 0; i < gameIndex; i++) {
    byte expectedButton = gameSequence[i];
    byte actualButton = readButtons();
    lightLedAndPlayTone(actualButton);
    if (expectedButton != actualButton) {
      return false;
    }
  }

  return true;
}

/**
   Plays a hooray sound whenever the user finishes a level
*/
void playLevelUpSound() {
  tone(SPEAKER_PIN, NOTE_E4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G4);
  delay(150);
  tone(SPEAKER_PIN, NOTE_E5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_C5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_D5);
  delay(150);
  tone(SPEAKER_PIN, NOTE_G5);
  delay(150);
  noTone(SPEAKER_PIN);
}

/**
   The main game loop
*/
void loop() {
  displayScore();

  // Add a random color to the end of the sequence
  gameSequence[gameIndex] = random(0, 4);
  gameIndex++;
  if (gameIndex >= MAX_GAME_LENGTH) {
    gameIndex = MAX_GAME_LENGTH - 1;
  }

  playSequence();
  if (!checkUserSequence()) {
    gameOver();
  }

  delay(300);

  if (gameIndex > 0) {
    playLevelUpSound();
    delay(300);
  }
}
