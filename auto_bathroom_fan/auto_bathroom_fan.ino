#include <Adafruit_NeoPixel.h>

#define FAN_PWM_PIN 9
#define WB_PIN1 A0
#define WB_PIN2 A1

#define FREQ 20
#define ON 255
#define OFF 0

#define ON_THRESH_V 200
#define SWITCH_THRESH_MS 200
#define CLAMP_MARGIN 10
#define RAMP_MULTIPLIER 1.0

#define LED_PIN 13
#define NUM_LEDS 8
#define RAINBOW_SPEED 300

Adafruit_NeoPixel leds(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void pwmInit() {
  DDRB |= (1 << PB1);                                    // pin 9 output
  TCCR1A = (1 << COM1A1) | (1 << WGM11);                 // non-inverting
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS10);    // fast PWM, /1
  ICR1 = 639;                                            // 16MHz / 640 = 25kHz
  OCR1A = 0;
}

void setup() {
  Serial.begin(9600);
  pwmInit();

  leds.begin();
  leds.show();

  pinMode(FAN_PWM_PIN, OUTPUT);
  pinMode(WB_PIN1, INPUT);
  pinMode(WB_PIN2, INPUT);
}

int state = OFF;
float pwmCmd = state;
int lastContinuousState = OFF;
unsigned long lastContinuousStartMs = millis();

void loop() {
  // read ldr
  int v1 = analogRead(WB_PIN1), v2 = analogRead(WB_PIN2);
  int vdiff = v1 - v2;

  int sensedState;
  if (vdiff < ON_THRESH_V) {
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

  // control
  float pwmDiff = state - pwmCmd;
  pwmCmd += RAMP_MULTIPLIER * pwmDiff / FREQ;
  if (pwmCmd <= CLAMP_MARGIN) pwmCmd = 0;
  if (pwmCmd >= 255-CLAMP_MARGIN) pwmCmd = 255;
  fanDuty(pwmCmd * 100.0f / 255.0f);

  Serial.print("pwmCmd: ");
  Serial.println(pwmCmd);

  // lights
  leds.setBrightness((int)(100*pwmCmd/255.0));
  rainbow();
  leds.show();

  delay(1000/FREQ);
}

inline void fanDuty(float pct) {
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
