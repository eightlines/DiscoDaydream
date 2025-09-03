#include <Arduino.h>
#include <lvgl.h>
#include "lcd.h"

void setup() {
  Serial.begin(115200);
  Serial.println("LVGL on SmartKnob");

  lcd_init();
}

void loop() {
  delay(5);
}
