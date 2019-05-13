/*
  rack lights and water
  This program runs automatic timers for lights and water in the rack system.

  lights
  Total there are three light channels by the rack level - top T, middle M and lower L

  lights are on a daily 24hr schedule to turn on and off to achieve a desired photoperiod.
  The light timing is specified by two parameters (1) start time and (2) ON duration (in hrs).
  OFF duration hrs is calculated as 24 - ON duration.

  [pinId, relayNO, start time, ON time, OFF time]

  pumps
  Two pump channels are available - A and B

  pumps operate by turning ON and OFF in sequence independent of the time of day.
  Pumps are specified by two parameters - ON time (seconds) and OFF time (seconds).

*/
#include <TimeLib.h>
#include <TimeAlarms.h>

// declare variables and functions
int Nlights = 2;
int Npumps = 2;
int lightPins[] = {12,10}; // {T, L}
int pumpPins[] = {9,8}; // {A, B}
int lightON = 7; // hr of day
int lightOFF = 19; // hr of the day
int sec_counter = 0;
int pump_STATUS = 0;
long int pumpON = 30; // seconds
long int pumpOFF = 1200; // seconds 20 min

long int MS_SEC = 1000; //milliseconds

void pumpsloop();
void lightsON();
void lightsOFF();

void setup() {
  pinMode(LED_BUILTIN,OUTPUT);  
  // declare pins as output
  for (int i = 0; i<Nlights; i++){
    pinMode(lightPins[i],OUTPUT);
  }
  for (int i = 0; i<Npumps; i++){
    pinMode(pumpPins[i],OUTPUT);
  }
    Serial.begin(9600);

  // manually reset the time - later will replce with Serial communication or use a clocking device 
  setTime(8,35,0,8,5,19); // set time to 16:40:00 Sun 05 May 2019
    
  if (hour()>=lightON && hour()<lightOFF) {
    // turn lights ON
    for (int i = 0; i<Nlights; i++){
      digitalWrite(lightPins[i], HIGH);
    }
  } else {
    // turn lights OFF
    for (int i = 0; i<Nlights; i++){
      digitalWrite(lightPins[i], LOW);
    }
  }

  // turn pumps OFF initially
  for (int i = 0; i<Npumps; i++){
  digitalWrite(pumpPins[i], HIGH);
  }

}

void loop() {
  pumpsloop();
  // digitalClockDisplay();
  // call the alarms, to trigger functions at specific times
  Alarm.alarmRepeat(lightON,0,0,lightsON);
  Alarm.alarmRepeat(lightOFF,0,0,lightsOFF);
  Alarm.delay(MS_SEC);
}

void lightsON() {
  for (int i = 0; i<Nlights; i++){
    digitalWrite(lightPins[i], HIGH);
  }
  //digitalWrite(LED_BUILTIN,HIGH);
  Serial.println("Turn light ON");
}

void lightsOFF() {
  for (int i = 0; i<Nlights; i++){
    digitalWrite(lightPins[i], LOW);
  }
  Serial.println("Turn light OFF");
}

void pumpsloop() {
  if (pump_STATUS == 0) {
    if (sec_counter>=pumpOFF) {
      // turn pumps ON
      for (int i = 0; i<Npumps; i++){
        digitalWrite(pumpPins[i], LOW);
      }
      // digitalWrite(LED_BUILTIN,HIGH);
      Serial.println("Turn pump ON");
      sec_counter = 0;
      pump_STATUS = 1;
    }
  } else {
    if (sec_counter>=pumpON) {
      // turn pumps OFF
      for (int i = 0; i<Npumps; i++){
        digitalWrite(pumpPins[i], HIGH);
      }
      // digitalWrite(LED_BUILTIN,LOW);
      Serial.println("Turn pump OFF");
      sec_counter = 0;
      pump_STATUS = 0;
    }
  }
  sec_counter = sec_counter + 1;
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.println();
}
void printDigits(int digits) {
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
