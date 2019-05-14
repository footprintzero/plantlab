#include <string.h>
#include <ArduinoJson.h>
#include <OneWire.h> 
#include <DallasTemperature.h>
#include <DHT.h>
#include <TimeLib.h>

//General
#define DHTTYPE DHT22   // DHT 22  (AM2302)
const long DEFAULT_TIME = 1357041600; // Jan 1 2013 00:00 - in seconds
String readingStr;
long int delaySEC = 3;
long int SEC_MS = 1000;
long int Wetread_MS = 2000;
long int DHTread_MS = 4000;
long int print_delay = 500;
long int rb_time;

// INPUT Signal
int pin_PH01 = A0;

//wet sensors
int pin_WET_T = 9;
int wetTCount = 3;
const int wetSensorCount = 1;
int wire_CTs[wetSensorCount] = {0};

// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(pin_WET_T); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

//air DHT sensors
const int DHTSensorCount = 4;
String DHTlabels[DHTSensorCount] = {"01","02","04","05"};
int DHTpins[DHTSensorCount] = {5,3,2,7};

// Initialize DHT sensor for normal 16mhz Arduino
//DHT DHT01(5,DHTTYPE);
DHT* DHTs[DHTSensorCount];

// OUTPUT Controls
int pin_FANL = 12;
int pin_FANR = 8;
int pin_PUMP = 4;
int pin_BYPL = 11;  //not used
int pin_BYPR = 10;  //not used

int devCount = 3;
int relayValue = HIGH;
long int dev_pins[3] = {pin_FANL, pin_FANR, pin_PUMP} ;
long int dev_status[3] = {1,1,1}; // FANL, FANR, PUMP. 0 = OFF, 1 = ON
long int counter[3][2] = {{0,0},{0,30},{0,0}}; // N x M 2D : N = #devices, M = 2 : ON , OFF
long int triggers[3][2] = {{2400,240},{30,2400},{12,4800}} ;// N x M 2D : N = #devices, M = 2 : ON time (sec), OFF time (sec)

// long int testtime_FANL = 300;
// long int testtime_FANR = 300;
// long int testtime_PUMP = 12;
// long int pausetime = 30;

void setup() {  

  // initialize wet sensors
  pinMode(pin_WET_T, INPUT);  
  sensors.begin();   // initialize Dallas sensor library

  // setup DHT readers and initialize pins
  initialize_DHTreaders();
  
  // setup controls set relay defaults
  initialize_controls();

 // open serial port
 Serial.begin(9600); 
 Serial.println("booting up");
}

void loop() {
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
  } 
  timer_mainloop();
}

void initialize_DHTreaders() {
  int i = 0;
  for (i;i<DHTSensorCount;i++){
    // initialize DHT pins?
    pinMode(DHTpins[i], INPUT);
    // instantiate the DHT reader object
    DHTs[i] = new DHT(DHTpins[i], DHTTYPE);
  }
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
  
  reading_wetTemps();  // read wet sensors
  Serial.print(",");
  reading_DHTs();  // read DHTs
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

void reading_wetTemps() {
  int i = 0;
  String readingStr;
  for (i;i<wetSensorCount;i++){
    // set the sensor id
    String sensorid = "CT0";
    sensorid += String(i+1);
    // read the sensor value
    sensors.requestTemperatures(); // Send the command to get temperature readings
    
    delay(Wetread_MS);     
    float value = sensors.getTempCByIndex(wire_CTs[i]);
    
    // create the json reading str    
    if (i==0) {
      readingStr = sensor_reading(sensorid,value);
    } else {
      readingStr += sensor_reading(sensorid,value);
    }
    if (i<(wetSensorCount-1)) {
      readingStr += ",";
    }
  }
  Serial.print(readingStr);
  delay(print_delay);  
}

void reading_DHTs() {
  int i = 0;
  String readingStr;
  for (i;i<DHTSensorCount;i++){
    Serial.print(reading_DHT(i,0));
    delay(print_delay);  
    Serial.print(",");
    Serial.print(reading_DHT(i,1));
    delay(print_delay);  
    
    if (i<(DHTSensorCount-1)) {
      Serial.print(",");
    }
  }
}

String reading_DHT(int sensor_index,int readtype) {
  String readingStr;
  
  // set the sensorid
  String sensorid;
  if (readtype==0) {
    sensorid = "TA";
  } else {
    sensorid = "HA";
  }
  sensorid += DHTlabels[sensor_index];

  // read from the sensor
  float value;
  DHTs[sensor_index]->begin();   
  if (readtype==0) {
    value= DHTs[sensor_index]->readTemperature();
  } else {
    value = DHTs[sensor_index]->readHumidity();
  }
  delay(DHTread_MS);

  //create the json reading string
  readingStr = sensor_reading(sensorid,value);
  return readingStr;
}

void initialize_controls() {
  // setup OUTPUT pins
  pinMode(pin_FANL, OUTPUT);
  pinMode(pin_FANR, OUTPUT);
  pinMode(pin_PUMP, OUTPUT);
  pinMode(pin_BYPL, OUTPUT);
  pinMode(pin_BYPR, OUTPUT);
  
  // initialize pin values
  digitalWrite(pin_FANL, relayValue);
  digitalWrite(pin_FANR, relayValue);
  digitalWrite(pin_PUMP, relayValue);
  digitalWrite(pin_BYPL, relayValue);
  digitalWrite(pin_BYPR, relayValue);
}

void timer_mainloop() {
  // deviceid is ambiguous, call all devices
  for (int i = 0; i<devCount;i++) {
    // call ambiguously for both ON and OFF triggers
    timer_deviceloop(i);
  }
  delay(SEC_MS);  
}

void timer_deviceloop(int deviceid) {
  timercall(deviceid,0); // call for ON trigger
  timercall(deviceid,1); // call for OFF trigger
}

void timercall(int deviceid, int ONOFF) {
  // device is specified, trigger action is specified
  
  // check device status
  int isOFF = dev_status[deviceid];
  int cnt = counter[deviceid][1-ONOFF];
  int trigger = triggers[deviceid][1-ONOFF];
  // check if device is not already in the call state
  if (isOFF!=ONOFF) {
    // check if device has reached its trigger point
    if (cnt>=trigger) {
      // call the action
      int callid = LOW;
      if (ONOFF==0) {
        callid = 1-relayValue;
      }else {
        callid = relayValue;
      }
      digitalWrite(dev_pins[deviceid], callid);      
      // change the state
      dev_status[deviceid] = 1-dev_status[deviceid];
      // reset the counter
      counter[deviceid][1-ONOFF] = 0;
    } else {
      // advance the counter
      counter[deviceid][1-ONOFF] = cnt  +1;
    }
  }
}
