#include <M5Core2.h>
#include <iKY040.h>

iKY040 encoder(13, 14, 26);  // CLK=13, DT=14, SW=26, direction by default

void drawStatus(const String& eventText) {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(20, 20);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.println("iKY040 + Core2");
  M5.Lcd.println();
  M5.Lcd.print("Position: ");
  M5.Lcd.println(encoder.getPosition());
  M5.Lcd.println();
  M5.Lcd.print("Event: ");
  M5.Lcd.println(eventText);
}

void setup() {
  M5.begin();
  Serial.begin(115200);

  encoder.begin(true, 4);
  drawStatus("---");
}

void loop() {
  M5.update();
  encoder.update();

  int32_t delta = encoder.getAndResetDelta();
  if (delta != 0) {
    drawStatus(delta > 0 ? "CW" : "CCW");
  }

  if (encoder.wasClicked()) {
    drawStatus("click");
  }

  delay(5);
}
