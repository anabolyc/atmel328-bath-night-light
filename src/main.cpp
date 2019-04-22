#include <Arduino.h>
#include <LowPower.h>
#include "main.h"

state current_state;

void white() {
  analogWrite(PIN_WHI, 0);
  analogWrite(PIN_RED, 0);
  analogWrite(PIN_GRN, 100);
  analogWrite(PIN_BLU, 100);
}

void setup() {
  pinMode(PIN_WHI, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BLU, OUTPUT);
  pinMode(PIN_GRN, OUTPUT);
  
  digitalWrite(PIN_WHI, OFF);
  digitalWrite(PIN_RED, OFF);
  digitalWrite(PIN_BLU, OFF);
  digitalWrite(PIN_GRN, OFF);

  pinMode(PIN_ON0, OUTPUT);
  pinMode(PIN_ON1, OUTPUT);

  pinMode(PIN_SNH, INPUT);

  current_state = BEFORE_ON;

  Serial.begin(9600);
  Serial.println("Hello, world!");

  // pinMode(LED_BUILTIN, OUTPUT);
  // digitalWrite(LED_BUILTIN, HIGH);
}

void enablePins(uint8_t state) {
  if (state == 1) {
    digitalWrite(PIN_ON0, HIGH);
    digitalWrite(PIN_ON1, HIGH);
  } else {

    digitalWrite(PIN_ON0, LOW);
    digitalWrite(PIN_ON1, LOW);
  }
}

void allOff() {
  digitalWrite(PIN_WHI, OFF);
  digitalWrite(PIN_RED, OFF);
  digitalWrite(PIN_GRN, OFF);
  digitalWrite(PIN_BLU, OFF);
}

void on(uint8_t pin) {
  for (uint8_t i = 0xff; i >= 0; i--) {
    analogWrite(pin, 0xff - i);
    delay(ON_DELAY >> 8);
  }
}

void on(uint8_t pin1, uint8_t pin2) {
  for (uint8_t i = 0xff; i >= 0; i--) {
    analogWrite(pin1, 0xff - i);
    analogWrite(pin2, 0xff - i);
    delay(ON_DELAY >> 8);
  }
}

void on(uint8_t pin1, uint8_t pin2, uint8_t pin3) {
  for (uint8_t i = 0; i < 0xff; i++) {
    analogWrite(pin1, 0xff - i);
    analogWrite(pin2, 0xff - i);
    analogWrite(pin3, 0xff - i);
    delay(ON_DELAY >> 8);
  }
}

uint16_t fps                    = 60;
uint16_t transitionDuration     = 3000;

int16_t frameIndex = -1;
uint16_t frameDuration = 1000 / fps;
int16_t transitionFrames = transitionDuration / frameDuration;

uint8_t lastColor[3] = {0, 0, 0};
uint8_t nextColor[3] = {0, 0, 0};
uint8_t currColor[3] = {0, 0, 0};
uint8_t COL_BLACK[] = {0, 0, 0};

void copyColor(uint8_t from[], uint8_t to[]) {
  for (int i = 0; i < 3; i++)
    to[i] = from[i];
}

void getRandomColor(uint8_t dest[]) {
  while (true) {
    uint8_t r = random(0xFF);
    uint8_t g = random(0xFF);
    uint8_t b = random(0xFF);
    uint16_t total = r + g + b;
    if (total > 0xFF && total <= 0xFF * 2) {
      uint8_t avg = total / 3;
      if (abs(avg - r) > 0xFF / 2 || abs(avg - g) > 0xFF / 2 || abs(avg - b) > 0xFF / 2) {
        dest[0] = r;
        dest[1] = g;
        dest[2] = b;
        Serial.print("getRandomColor: ");
        Serial.print(r);
        Serial.print(' ');
        Serial.print(g);
        Serial.print(' ');
        Serial.print(b);
        Serial.print('\n');
        break;
      }
    }
  }
}

void setColor(uint8_t color[]) {
  analogWrite(PIN_RED, 0xff - color[0]);
  analogWrite(PIN_GRN, 0xff - color[1]);
  analogWrite(PIN_BLU, 0xff - color[2]);
}

void getCurrentColor(uint8_t prev[], uint8_t next[], uint8_t dest[], uint16_t frameIndex, uint16_t frameCount) {
  for ( int i = 0; i < 3; i++) {
    uint8_t prev0 = prev[i];
    uint8_t next0 = next[i];
    uint8_t curr0 = 0;
    if (next0 >= prev0)
      curr0 = prev0 + (next0 - prev0) * frameIndex / frameCount;
    else
      curr0 = prev0 - (prev0 - next0) * frameIndex / frameCount;
    dest[i] = curr0;
  }
}

state last_visibe_state;
uint8_t power_led_state = 0;

void check_power_off() {
  uint16_t sens = analogRead(PIN_SNH);
  if (sens < 512) {
    enablePins(0);
    allOff();
    Serial.println(" -> POWER_GONE");
    current_state = POWER_GONE;

    copyColor(COL_BLACK, nextColor);
    frameIndex = -1;
  }
}

void check_power_on() {
  uint16_t sens = analogRead(PIN_SNH);
  if (sens >= 512) {
    enablePins(1);
    if (last_visibe_state == WHITE_ON) {
      Serial.println(" -> RAINBOW_ON");
      current_state = RAINBOW_ON;
      last_visibe_state = RAINBOW_ON;
    } else {
      Serial.println(" -> BEFORE_ON");
      current_state = BEFORE_ON;
    }
  }
}

uint8_t state = 1;

void loop() {
  
  switch (current_state) {
    case BEFORE_ON:    
      enablePins(1);
      white();
      Serial.println(" -> WHITE_ON");
      current_state = WHITE_ON;
      last_visibe_state = WHITE_ON;
      break;

    case WHITE_ON: 
      check_power_off();
      break;

    case POWER_GONE: 
      digitalWrite(LED_BUILTIN, power_led_state);
      LowPower.powerDown(power_led_state ? SLEEP_15MS : SLEEP_500MS, ADC_OFF, BOD_OFF);
      power_led_state = !power_led_state;
      check_power_on();
      break;

    case RAINBOW_ON:
      if (frameIndex >= transitionFrames || frameIndex == -1) {
        frameIndex = 0;
        copyColor(nextColor, lastColor);
        getRandomColor(nextColor);
      }

      getCurrentColor(lastColor, nextColor, currColor, frameIndex, transitionFrames);
      setColor(currColor);

      frameIndex++;
      delay(frameDuration);
      check_power_off();
      break;
  }

  // digitalWrite(LED_BUILTIN, state);
  // state = !state;
  // delay(1000);
}