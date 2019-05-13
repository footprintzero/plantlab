/*
 * carbon dioxide sensor
 * This script is to test the operation of the carbon dioxide sensor MHZ-14A
 * which works on the principle of NDIR.
 * 
 * The hookup for the MHZ-14A is pin01-5V, pin02-GND and pin06-PWM (data signal)
*/

// import module for Serial communication
#include <SoftwareSerial.h>

// declare variables
// const int analogPin = A0;
const int pwmPin = 9;
const long samplePeriod = 10000L;

SoftwareSerial sensor(10, 11); // RX, TX
const byte requestReading[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte result[9];
long lastSampleTime = 0;

void setup() {
  Serial.begin(9600);
  sensor.begin(9600);
  pinMode(pwmPin, INPUT_PULLUP);
}

void loop() {
  long now = millis();
  if (now > lastSampleTime + samplePeriod) {
    lastSampleTime = now;
    int ppmPWM = readPPMPWM();
    Serial.print("\n"); 
    Serial.print(ppmPWM); 
  }
}

int readPPMPWM() {
  while (digitalRead(pwmPin) == LOW) {}; // wait for pulse to go high
  long t0 = millis();
  while (digitalRead(pwmPin) == HIGH) {}; // wait for pulse to go low
  long t1 = millis();
  while (digitalRead(pwmPin) == LOW) {}; // wait for pulse to go high again
  long t2 = millis();
  long th = t1-t0;
  long tl = t2-t1;
  long ppm = 5000L * (th - 2) / (th + tl - 4);
  while (digitalRead(pwmPin) == HIGH) {}; // wait for pulse to go low
  delay(10); // allow output to settle.
  return int(ppm);
}
