#include <string.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>

// pins
long int fan_pins[2][2] = {{3,2},{13,12}}; // chamber x wall 2D : {right chmb {right,back},left chmb {left,back}}}
long int man_pins[2] = {7,8}; // right, left chamber
long int mhz_pins[2] = {10,11}; //left, right

// package for CO2 sensor reading
SoftwareSerial sensor(10, 11); // RX, TX
const int mhzCount = 2;
const long samplePeriod = 60000L; //MHZ sampling time in ms
const byte requestReading[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte result[9];
long lastSampleTime = 0;

// string json and timestamps
const long DEFAULT_TIME = 1357041600; // Jan 1 2013 00:00 - in seconds
String readingStr;
long int print_delay = 500;
long int rb_time;


void setup() {
  set_fans();
  setup_mhz();
  //set_manifold();
}


void loop() {
  sensor_reading();
}

void setup_mhz() {
  Serial.begin(9600);
  sensor.begin(9600);
  pinMode(mhz_pins[0], INPUT_PULLUP);
  pinMode(mhz_pins[1], INPUT_PULLUP);
}

void sensor_reading() {
  if (Serial.available()) {
    String rbpiStr = Serial.readString();
    rbpiStr.replace("\n","");
    DynamicJsonDocument rbpiJson(300);
    deserializeJson(rbpiJson,rbpiStr);

    long int timeserial = rbpiJson["timeserial"];
    syncTime(timeserial);

    Serial.print("{\"message_type\":\"sensor_reading\",\"readings\":");
    delay(print_delay);
    reading_allsensors();
    Serial.print("}");
    Serial.println();
    Serial.flush();
    delay(samplePeriod);
  }
}

void set_fans() {
  setup_fan(0,0,HIGH);
  setup_fan(0,1,HIGH);

  setup_fan(1,0,LOW);
  setup_fan(1,1,LOW);
}

void set_manifold() {
  setup_manifold(0,HIGH);
  setup_manifold(1,HIGH);
}

void setup_fan(long int chbid,long int wallid,int ONOFF) {
  long int pinid = fan_pins[chbid][wallid];
  pinMode(pinid, OUTPUT);
  digitalWrite(pinid,ONOFF);
}

void setup_manifold(long int mfid,int ONOFF) {
  long int pinid = man_pins[mfid];
  pinMode(pinid, OUTPUT);
  digitalWrite(pinid,ONOFF);
}

void syncTime(long int timeserial) {
  if (timeserial>0) {
    setTime(timeserial);
  }else {
    setTime(DEFAULT_TIME);
    // setTime(7,0,0,5,1,19); // another way to manually set time 07:00 Jan 05 2019
  }
}

char* currentTime() {
  static char timechar[50];
  sprintf(timechar,"%02d/%02d/%04d %02d:%02d:%02d",day(),month(),year(),hour(),minute(),second());
  // String timeStr = String(timechar);
  return timechar;
}

void reading_allsensors() {
  Serial.print("[");
  // load the readings in here :

  reading_mhz();  // read wet sensors
  // Serial.print(",");
  // read something else
  Serial.print("]"); // cap it off
}

String sensor_reading(String sensorid, float value) {
  String jsonStr;
  char* timechar = currentTime();
  StaticJsonDocument<300> doc;
  doc["sensorid"] = sensorid;
  doc["datetime"] = timechar;
  doc["value"] = value;
  serializeJson(doc,jsonStr);
  return jsonStr;
}

void reading_mhz() {
  int i = 0;
  String readingStr;
  for (i;i<mhzCount;i++){
    // set the sensor id
    String sensorid = "CD0";
    sensorid += String(i+1);
    // read the sensor value
    float value = readPPMPWM(mhz_pins[i]);;

    // create the json reading str
    if (i==0) {
      readingStr = sensor_reading(sensorid,value);
    } else {
      readingStr += sensor_reading(sensorid,value);
    }
    if (i<(mhzCount-1)) {
      readingStr += ",";
    }
  }
  Serial.print(readingStr);
  delay(print_delay);
}
