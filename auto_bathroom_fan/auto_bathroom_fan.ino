#include <Adafruit_NeoPixel.h>

// Pin defs
#define PD0 0
#define PD1 1
#define PD2 2
#define PD3 3
#define PD4 4
#define PD5 5
#define PD6 6
#define PD7 7
#define PB0 8
#define PB1 9
#define PB2 10
#define PB3 11
#define PB4 12
#define PB5 13

#define PC0 A0
#define PC1 A1
#define PC2 A2
#define PC3 A3
#define PC4 A4
#define PC5 A5
#define ADC6 A6
#define ADC7 A7


// Net defs
#define WB_1 PC0
#define WB_2 PC1
#define DEBUG_LED PB2
#define FAN_PWM PB1
#define FAN_LEDS PB0
#define FAN_TACH PD2
#define FAN_EN PD7
#define EN_SW PD4


// Params
#define FREQ 20
#define ON 255
#define OFF 0

#define ON_THRESH 300
#define SWITCH_THRESH_MS 200
#define CLAMP_MARGIN 5
#define RAMP_MULTIPLIER 1.0

#define NUM_LEDS 8
#define RAINBOW_SPEED 300

Adafruit_NeoPixel leds(NUM_LEDS, FAN_LEDS, NEO_GRB + NEO_KHZ800);

void pwmInit() {
  DDRB |= (1 << FAN_PWM);                                // output pin
  TCCR1A = (1 << COM1A1) | (1 << WGM11);                 // non-inverting
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);    // fast PWM, /1
  ICR1 = 639;                                            // 16MHz / 640 = 25kHz
  OCR1A = 0;
}

void setup() {
  Serial.begin(9600);
  pwmInit();

  pinMode(WB_1, INPUT);
  pinMode(WB_2, INPUT);
  pinMode(DEBUG_LED, OUTPUT);
  pinMode(FAN_PWM, OUTPUT);
  pinMode(FAN_LEDS, OUTPUT);
  pinMode(FAN_TACH, INPUT);
  pinMode(FAN_EN, OUTPUT);
  pinMode(EN_SW, INPUT);

  leds.begin();
  leds.show();

}

int state = OFF;
float pwmCmd = state;
int lastContinuousState = OFF;
unsigned long lastContinuousStartMs = millis();

void loop() {
  int wb1 = analogRead(WB_1);
  int wb2 = analogRead(WB_2);
  int diff = wb1-wb2;
  
  int sensedState;
  if (diff < ON_THRESH) {
    sensedState = OFF;
  } else {
    sensedState = ON;
  }

  // debounce logic
  unsigned long now = millis();
  if (sensedState != lastContinuousState) {
    lastContinuousState = sensedState;
    lastContinuousStartMs = now;
  } else if (now - lastContinuousStartMs >= SWITCH_THRESH_MS) {
    state = sensedState;
    lastContinuousStartMs = now;
  }

  // fan speed update
  float pwmDiff = state - pwmCmd;
  pwmCmd += RAMP_MULTIPLIER * pwmDiff / FREQ;
  if (pwmCmd <= CLAMP_MARGIN) pwmCmd = 0;
  if (pwmCmd >= 255-CLAMP_MARGIN) pwmCmd = 255;

  Serial.print("pwmCmd: ");
  Serial.println(pwmCmd);

  // debug LED
  analogWrite(DEBUG_LED, pwmCmd);

  // light/fan control
  int en_sw = digitalRead(EN_SW);
  if (en_sw == HIGH) {
    fanDuty(pwmCmd * 100.0f / 255.0f);
    leds.setBrightness((int)(100*pwmCmd/255.0));
    rainbow();
    leds.show();
  } else {
    fanDuty(0);
    leds.clear();
    leds.show();
  }

  delay(1000/FREQ);

}

inline void fanDuty(float pct) {
  if (pct == 0) {
    digitalWrite(FAN_EN, LOW);
  } else {
    digitalWrite(FAN_EN, HIGH);
  }
  if (pct < 0)   pct = 0;
  if (pct > 100) pct = 100;
  OCR1A = (uint16_t)(pct * 6.39f);
}

static uint16_t offset = 0;
void rainbow() {

  for (int i = 0; i < NUM_LEDS; i++) {
    // Spread the rainbow across the LEDs
    uint16_t hue = offset + (i * 65536L / NUM_LEDS);

    leds.setPixelColor(i, leds.gamma32(leds.ColorHSV(hue)));
  }

  leds.show();

  offset += RAINBOW_SPEED*(int)(256.0/FREQ);
}
