#define SH_CP 16
#define ST_CP 14
#define DS    12
#define EN    13

#define latchPin ST_CP
#define clockPin SH_CP
#define dataPin DS

#define drinkingWaterMotorRelay B01000000
#define drinkingWaterMotorPin   1
#define otherWaterMotorRelay    B00100000
#define otherWaterMotorPin      2
#define light                   B00010000
#define lightPin                3

int data = B00000000;

void setup() {
  pinMode(EN, OUTPUT);
  digitalWrite(EN, LOW);      //donot read the noise - 74hc595
  pinMode(SH_CP, OUTPUT);
  pinMode(ST_CP, OUTPUT);
  pinMode(DS, OUTPUT);
  delay(200);
  digitalWrite(EN, HIGH);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  data |= otherWaterMotorRelay;
  digitalWrite(LED_BUILTIN, 1);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
  delay(500);
  data &= ~(otherWaterMotorRelay);
  digitalWrite(LED_BUILTIN, 0);
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, data);
  digitalWrite(latchPin, HIGH);
  delay(500);
}
