#include <M5StickC.h>

void calibrateSensor() {
  unsigned char cmd[] = {0xff, 0x01, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78};
  for (int i = 0; i < sizeof(cmd); i++) {
    Serial1.write(cmd[i]);
  }
}

int getSensorData() {
  int j = 0;
  int co2;
  static int buf[9];
  unsigned char cmd[] = {0xff, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
  for (int i = 0; i < sizeof(cmd); i++) {
    Serial1.write(cmd[i]);
  }
  delay(200);
  while (Serial1.available() > 0) {
    int val = Serial1.read();
    buf[j++] = val;
    // print debug info
    M5.Lcd.setTextColor(RED, BLACK);
    M5.Lcd.setCursor(1, 64);
    M5.Lcd.printf("%02x%02x%02x%02x %02x%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[8]);
    if (j == 9) {
      j = 0;
      if (buf[0] == 0xff && buf[1] == 0x86) {
        co2 = buf[2] * 256 + buf[3];
        break;
      }
    }
  }
  return co2;
}

void displayCo2(int co2) {
  static int flag = 0;
  // Display: CO2: 5000 ppm
  M5.Lcd.setCursor(1, 0);
  if (0 <= co2 && co2 < 1000) { // Normal
    M5.Lcd.setTextColor(BLACK, GREEN);
  } else if (1000 <= co2 && co2 < 2000) { // Poor concentration
    M5.Lcd.setTextColor(BLACK, YELLOW);
  } else if (2000 <= co2 && co2 < 3000) { // Headache
    M5.Lcd.setTextColor(BLACK, ORANGE);
  } else if (3000 <= co2) { // No concentration
    M5.Lcd.setTextColor(BLACK, RED);
  }
  if (flag == 0) {
    flag = 1;
    M5.Lcd.printf("CO2:%4d ppm%c", co2, '#');
  } else {
    flag = 0;
    M5.Lcd.printf("CO2 %4d ppm%c", co2, '#');
  }
}

#define SCREEN_WIDTH 160
void displayCo2History(int co2) {
  static int history_timer = 0;
  static int offset = 0;
  static int history_buf[SCREEN_WIDTH];
  if (history_timer++ >= 180) {
    history_timer = 0;
    history_buf[offset++] = co2;
    offset = offset % SCREEN_WIDTH;
    M5.Lcd.fillRect(0, 20, SCREEN_WIDTH, 40, BLACK);
    for (int x = 0; x < SCREEN_WIDTH; x++){
      int y = 59 - ( history_buf[(offset + x) % SCREEN_WIDTH]) / 50;
      M5.Lcd.drawPixel(x, y, WHITE);
    }
  }
}

void buttonOperation() {
  static int screen_timer = 10;
  // Wait 1 sec with button operation
  for (int i = 0; i < 10; i++) {
    // Home: screen brightness
    if (digitalRead(M5_BUTTON_HOME) == LOW) {
      // Brighten the screen
      screen_timer = 10;
      M5.Axp.ScreenBreath(10);
      while (digitalRead(M5_BUTTON_HOME) == LOW);
    }
    // Reset: calibration under CO2 400ppm condition
    if (digitalRead(M5_BUTTON_RST) == LOW) {
      calibrateSensor();
      while (digitalRead(M5_BUTTON_RST) == LOW);
    }
    delay(100); // ms
  }
  // Dim the screen
  if (screen_timer-- == 0) {
    screen_timer = 10;
    M5.Axp.ScreenBreath(7);
  }
}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Axp.ScreenBreath(10);
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);
  // GND, 5V0, G26, G36(Tx), G00(Rx), BAT, 3V3, VIN
  Serial1.begin(9600, SERIAL_8N1, 36, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
  int co2;
  co2 = getSensorData();
  displayCo2(co2);
  displayCo2History(co2);
  buttonOperation();
}
