#include "DHT.h"
#include <Servo.h>  // include servo library to use its related functions
#include <LiquidCrystal.h>

#define DHTTYPE DHT11  // DHT11

#define FUNCTION_PIN 11
#define UP_BUTTON_PIN 10
#define DHTPIN 13
#define DOWN_BUTTON_PIN 12
#define Servo_PWM 7  // A descriptive name for D6 pin of Arduino to provide PWM signal
#define RELAY_PIN 8

Servo myServo;       // Define an instance of of Servo with the name of "MG995_Servo"
DHT dht(DHTPIN, DHTTYPE);
int servo_counter = 0;
int lastWindowAction = 0;
int servo_start_position = 130;
// Function relate
int function_button_before = 0;
int function_button_count = 0;
int current_function = 0;

// Targets
double target_temp = 23;
int target_humi = 80;

// Up down buttons
int down_button_before = 1;
int up_button_before = 1;

// States
int lampOnPrev = 0;
int windowOpenPrev = 0;
float bound = 0.3;
float humiBound = 3;

// Initialiseer LCD met de pennen zoals we hem hebben aangesloten
// LiquidCrystal lcd(44, 42, 40, 38, 36, 34);
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

int getFunction(int function_count) {
  if (function_count % 2 == 0) {
    return 0;
  } else {
    return 1;
  }
}

void setup() {
  pinMode(FUNCTION_PIN, INPUT_PULLUP);
  pinMode(DOWN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(UP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  Serial.begin(9600);
  lcd.begin(16, 2);
  dht.begin();
  openWindow();
  closeWindow();
}


// free RAM check for debugging. SRAM for ATmega328p = 2048Kb.
int availableMemory() {
    // Use 1024 with ATmega168
    int size = 8192;
    byte *buf;
    while ((buf = (byte *) malloc(--size)) == NULL);
        free(buf);
    return size;
}
void loop() {
  delay(100);
  lastWindowAction++;
  if (lastWindowAction < 25){
    return;
  } else if (lastWindowAction == 25){
    // After 25*100ms, detach
      // myServo.detach();  // Connect D6 of Arduino with PWM signal pin of servo motor
      return;
  }
  // Else continue

  // Set to 0, because of pullup
  if ((function_button_before == 1) && (digitalRead(FUNCTION_PIN) == 0)) {
    function_button_count += 1;
    current_function = getFunction(function_button_count);
    Serial.println("Function: " + String(current_function));
  }
  function_button_before = digitalRead(FUNCTION_PIN);

  // Note: this is relative humidity
  float humi = dht.readHumidity();
  float tempC = dht.readTemperature();

  // Make a correction .Roughly off by 0.5 degree celsius
  tempC = tempC - 0.5;

  /*
  TODO: Correct RH
  For every 1 degree Celsius increase in temperature, the air's capacity to hold moisture increases by about 7%. Therefore, if the relative humidity of the air is initially 50% at a temperature of 20 degrees Celsius, and the temperature increases to 21 degrees Celsius, the relative humidity would decrease to approximately 46%. Similarly, if the temperature were to decrease to 19 degrees Celsius, the relative humidity would increase to approximately 54%.
  It's worth noting, however, that this is just an approximation and the actual change in relative humidity will depend on a variety of factors, such as the initial temperature and relative humidity, the amount of moisture in the air, and the specific conditions of the environment.
  */

  // Temp function
  if (current_function == 0) {
    if ((up_button_before == 1) && (digitalRead(UP_BUTTON_PIN) == 0)) {
      target_temp += 0.5;
      Serial.print(target_temp);
    }
    up_button_before = digitalRead(UP_BUTTON_PIN);


    if ((down_button_before == 1) && (digitalRead(DOWN_BUTTON_PIN) == 0)) {
      target_temp -= 0.5;
      Serial.print(target_temp);
    }
    down_button_before = digitalRead(DOWN_BUTTON_PIN);
  }

  // RH function
  if (current_function == 1) {
    if ((up_button_before == 1) && (digitalRead(UP_BUTTON_PIN) == 0)) {
      target_humi += 3;
    }
    up_button_before = digitalRead(UP_BUTTON_PIN);


    if ((down_button_before == 1) && (digitalRead(DOWN_BUTTON_PIN) == 0)) {
      target_humi -= 3;
    }
    down_button_before = digitalRead(DOWN_BUTTON_PIN);
  }



  // If it was previously on
  if (lampOnPrev == 1) {
    // Stay on until the tamp is larger than 0.5 above target
    if (tempC >= target_temp + bound && lastWindowAction > 50) {
      digitalWrite(RELAY_PIN, LOW);
      lampOnPrev = 0;
    }
  }
  // If it was previoulsy off
  else {
    if (tempC <= target_temp - bound && lastWindowAction > 50) {
      digitalWrite(RELAY_PIN, HIGH);
      lampOnPrev = 1;
    }
  }
  Serial.println("Target RH: " + String(target_humi) + " humi: " + String(humi));
  // If it previously was closed, only open if we are above the bound
  if (windowOpenPrev == 0 && lastWindowAction > 50) {
    if (humi > target_humi + humiBound) {
      openWindow();
    }
  }
  // If it was open, only close it if we are below the bound
  else {
    if (humi < target_humi - humiBound && lastWindowAction > 50) {
      closeWindow();
    }
  }

  // Relative humidity, change it by 7%
  lcd.setCursor(0, 0);
  lcd.print("Temp: " + String(tempC, 1));
  lcd.setCursor(11, 0);
  lcd.print(String(target_temp));
  lcd.setCursor(0, 1);
  lcd.print("Humi: " + String(int(humi)));
  lcd.setCursor(13, 1);
  lcd.print(String(target_humi));

  lcd.setCursor(15, current_function);
  lcd.print("*");
  lcd.setCursor(15, (current_function + 1) % 2);
  lcd.print(" ");

  lcd.setCursor(0,1);
  lcd.print(String(availableMemory()));

  // SEensor error handling
  if (isnan(humi) || isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
}

void openWindow() {
  // Note: 0 is opened
  // myServo.attach(Servo_PWM);  // Connect D6 of Arduino with PWM signal pin of servo motor
  // myServo.write(25);
  // windowOpenPrev = 1;
  // lastWindowAction =0;
}

void closeWindow() {
  // Note: 0 is opened
  // Serial.println("Close Window");
  // myServo.attach(Servo_PWM);  // Connect D6 of Arduino with PWM signal pin of servo motor
  // myServo.write(servo_start_position);
  // windowOpenPrev = 0;
  // lastWindowAction = 0;
}