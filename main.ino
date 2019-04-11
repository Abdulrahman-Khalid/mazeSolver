const byte pin1Motor1 = 10;
const byte pin2Motor1 = 11;
const byte pinSpeedMotor1 = 3;

const byte pin1Motor2 = 12;
const byte pin2Motor2 = 13;
const byte pinSpeedMotor2 = 9;

const byte motorSpeed = 100;
void setup() {
  // put your setup code here, to run once:
  pinMode(pin1Motor1, OUTPUT);
  pinMode(pin2Motor1, OUTPUT);
  pinMode(pinSpeedMotor1, OUTPUT);
   pinMode(pin1Motor2, OUTPUT);
  pinMode(pin2Motor2, OUTPUT);
  pinMode(pinSpeedMotor2, OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(pin1Motor1, HIGH);
  digitalWrite(pin2Motor1, LOW);

  digitalWrite(pin1Motor2, HIGH);
  digitalWrite(pin2Motor2, LOW);

  analogWrite(pinSpeedMotor1, motorSpeed);
  analogWrite(pinSpeedMotor2, motorSpeed);
}

void forward() {
  digitalWrite(pin1Motor1, HIGH);
  digitalWrite(pin2Motor1, LOW);

  digitalWrite(pin1Motor2, HIGH);
  digitalWrite(pin2Motor2, LOW);

  analogWrite(pinSpeedMotor1, motorSpeed);
  analogWrite(pinSpeedMotor2, motorSpeed);
}

void left() {
  digitalWrite(pin1Motor1, HIGH);
  digitalWrite(pin2Motor1, LOW);
}

void right() {
  digitalWrite(pin1Motor2, HIGH);
  digitalWrite(pin2Motor2, LOW);
}
