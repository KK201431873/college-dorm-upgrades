#include <Wire.h>
#include <Servo.h>
#include "RTClib.h"

RTC_DS3231 rtc;
Servo light;

#define SERVO_PIN 3
#define ON 115
#define OFF 60

void setup() {
  Serial.begin(9600);

  // Check if the RTC is connected properly
  if (!rtc.begin()) {
    Serial.println("Could not find RTC");
    while (1);
  }

  // Uncomment the line below ONCE if the power was lost and you need to set the time
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))+TimeSpan(7)); // +7s for upload delay
}

int last_angle = -1;
void loop() {
  DateTime now = rtc.now();

  // Print date
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");

  // Print time
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  int hour = now.hour();
  int minute = now.minute();
  int desired_angle;
  if ((23 <= hour && 30 <= minute) || (hour <= 7)) {
    desired_angle = OFF;
  } else {
    desired_angle = ON;
  }

  // int sec = now.second(), desired_angle;
  // if (0 < sec && sec <= 30) {
  //   desired_angle = OFF;
  // } else {
  //   desired_angle = ON;
  // }

  if (last_angle != desired_angle) {
    light.attach(SERVO_PIN);
    light.write(desired_angle);
    delay(1000);
    light.detach();
    last_angle = desired_angle;
  }

  delay(1000); // Wait 1 second before reading again
}
