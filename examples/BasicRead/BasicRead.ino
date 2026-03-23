#include <Arduino.h>
#include <iKY040.h>

iKY040 encoder;  // CLK=13, DT=14, SW=26, direction inverted by default

void setup() {
  Serial.begin(115200);
  delay(300);

  encoder.begin(true, 4);  // pull-up where supported, 4 transitions per detent
  encoder.setButtonDebounce(30);

  Serial.println();
  Serial.println("iKY040 ready");
  Serial.printf("CLK=%u, DT=%u, SW=%u\n", encoder.getPinCLK(), encoder.getPinDT(), encoder.getPinSW());
  Serial.println("Turn the encoder or press the button.");
}

void loop() {
  encoder.update();

  int32_t delta = encoder.getAndResetDelta();
  if (delta != 0) {
    Serial.print("Delta: ");
    Serial.print(delta);
    Serial.print(" | Position: ");
    Serial.println(encoder.getPosition());
  }

  if (encoder.wasPressed()) {
    Serial.println("Button pressed");
  }

  if (encoder.wasReleased()) {
    Serial.println("Button released");
  }

  if (encoder.wasClicked()) {
    Serial.println("Click detected");
  }

  delay(1);
}
