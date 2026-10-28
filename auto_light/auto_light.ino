// Автоосвещение, Arduino Uno + LDR (Тутов И., ИС-45к)
#include <Adafruit_LiquidCrystal.h>

const int LDR_PIN = A0;
const int BUTTON_PIN = 2;  // force lamp, INPUT_PULLUP
const int LED_GREEN = 3;   // день
const int LED_YELLOW = 4;  // сумерки
const int LED_RED = 5;     // ночь
const int BUZZER_PIN = 7;
const int LAMP_PIN = 9;    // ШИМ лампа
const int DAY_LEVEL = 700;
const int DUSK_LEVEL = 350;
const int STATE_DAY = 0, STATE_DUSK = 1, STATE_NIGHT = 2;
int currentState = STATE_DAY, lastState = STATE_DAY;
bool forceLamp = false;
unsigned long lastBeep = 0, lastLcd = 0;
Adafruit_LiquidCrystal lcd(0);

int readLightAdc() {
  long sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += analogRead(LDR_PIN);
    delay(2);
  }
  return sum / 10;
}

int getState(int adc) {
  if (adc >= DAY_LEVEL) return STATE_DAY;
  if (adc >= DUSK_LEVEL) return STATE_DUSK;
  return STATE_NIGHT;
}

void setLeds(int s) {
  digitalWrite(LED_GREEN, s == STATE_DAY ? HIGH : LOW);
  digitalWrite(LED_YELLOW, s == STATE_DUSK ? HIGH : LOW);
  digitalWrite(LED_RED, s == STATE_NIGHT ? HIGH : LOW);
}

int setLamp(int s, bool forced) {
  int pwm = 0;
  if (forced) pwm = 255;
  else if (s == STATE_DAY) pwm = 0;
  else if (s == STATE_DUSK) pwm = 90;
  else pwm = 255;
  analogWrite(LAMP_PIN, pwm);
  return pwm;
}

void handleBuzzer(int s) {
  if (s == lastState) return;
  unsigned long now = millis();
  if (now - lastBeep < 200) return;
  if (s == STATE_DAY) tone(BUZZER_PIN, 900, 80);
  else if (s == STATE_DUSK) tone(BUZZER_PIN, 1200, 80);
  else tone(BUZZER_PIN, 1600, 120);
  lastBeep = now;
}

void showOnLcd(int adc, int s, int pwm, bool forced) {
  int percent = constrain(map(adc, 0, 1023, 0, 100), 0, 100);
  lcd.setCursor(0, 0);
  lcd.print("Light: ");
  lcd.print(percent);
  lcd.print("%   ");
  lcd.setCursor(0, 1);
  if (forced) lcd.print("Mode: FORCE ON  ");
  else if (s == STATE_DAY) lcd.print("Mode: DAY       ");
  else if (s == STATE_DUSK) lcd.print("Mode: DUSK      ");
  else lcd.print("Mode: NIGHT     ");
  lcd.setCursor(15, 1);
  if (pwm > 200) lcd.print("*");
  else if (pwm > 0) lcd.print("+");
  else lcd.print(" ");
}

void handleButton() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    forceLamp = !forceLamp;
    tone(BUZZER_PIN, forceLamp ? 1800 : 700, 100);
    while (digitalRead(BUTTON_PIN) == LOW) delay(10);
    delay(50);
  }
}
void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LAMP_PIN, OUTPUT);
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setBacklight(1);
  lcd.print("AutoLight v1.0");
  digitalWrite(LED_GREEN, HIGH); delay(200);
  digitalWrite(LED_YELLOW, HIGH); delay(200);
  digitalWrite(LED_RED, HIGH); delay(200);
  analogWrite(LAMP_PIN, 180); delay(200);
  tone(BUZZER_PIN, 1400, 120); delay(200);
  analogWrite(LAMP_PIN, 0);
  setLeds(STATE_DAY);
  lcd.clear();
}

void loop() {
  int adc = readLightAdc();
  currentState = getState(adc);
  handleButton();
  handleBuzzer(currentState);
  if (currentState != lastState) {
    Serial.print(lastState); Serial.print("->"); Serial.println(currentState);
    lastState = currentState;
  }
  setLeds(forceLamp ? STATE_NIGHT : currentState);
  int pwm = setLamp(currentState, forceLamp);
  if (millis() - lastLcd >= 250) {
    showOnLcd(adc, currentState, pwm, forceLamp);
    Serial.println(adc);
    lastLcd = millis();
  }
}
