#define WB_PIN1 A0
#define WB_PIN2 A1
#define MOTOR 3

#define FREQ 20
#define ON 255
#define OFF 0

#define ON_THRESH_V 200
#define SWITCH_THRESH_MS 200
#define CLAMP_MARGIN 10
#define RAMP_MULTIPLIER 1.0

void setup() {
  Serial.begin(9600);

  pinMode(WB_PIN1, INPUT);
  pinMode(WB_PIN2, INPUT);
  pinMode(MOTOR, OUTPUT);
}

int state = OFF;
float pwmCmd = state;
int lastContinuousState = OFF;
int lastContinuousStartMs = millis();

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
  int now = millis();
  if (sensedState != lastContinuousState) {
    lastContinuousState = sensedState;
    lastContinuousStartMs = now;
  } else if (now - lastContinuousStartMs >= SWITCH_THRESH_MS) {
    state = sensedState;
    lastContinuousStartMs = now;
  }

  // control
  int pwmDiff = state - pwmCmd;
  pwmCmd += (float)RAMP_MULTIPLIER * pwmDiff / FREQ;
  if (pwmCmd <= CLAMP_MARGIN) pwmCmd = 0;
  if (pwmCmd >= 255-CLAMP_MARGIN) pwmCmd = 255;
  analogWrite(MOTOR, (int)pwmCmd);

  Serial.print("pwmCmd: ");
  Serial.println(pwmCmd);

  delay(1000/FREQ);
}
