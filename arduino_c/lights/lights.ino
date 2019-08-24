/*
  lights
  This program runs a test for lights in left and right chamber
  with variable dimming control for 3 channels - Blue, White and Red in each chamber
  The control range is 10-80%, so turndown capability is 12.5%.

  In order to turn off an individual channel, the relay needs to be 
  activated to close the circuit and force the voltage to 0V.

  In order to turn off all channels, the main power supply should be turned off for the chamber.

  The program runs through each chamber and each channel
  first turning ON the channel, then dimming, and then turning OFF the channel.
*/

#include <string.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>

// declare variables and functions
int Ncmb = 2;
int Nchn = 3;
int powerPin[] = {6,5};
int relayPin[][3] = {{13,12,8},{7,4,2}};
int dimPin[][3] = {{11,10,9},{14,15,16}};
int relayValue =LOW;

int devCount = 1;
long int dev_ids[6][2] = {{0,0},{0,1},{0,2},{1,0},{1,1},{1,2}};
long int dev_status[3] = {1,1,1}; //
long int stg_status[2] = {1,0}; //
int SEC_COUNTER = 0;
long int counter[2][4] = {{0,4,0,0},{0,4,0,0}}; // N = chamber, M = (night, morn, afternoon, evening)
long int triggers[2][4] = {{12,6,4,2},{12,6,4,2}} ;// N = chamber,  M = (night, morn, afternoon, evening)
long int dim_lvl[3][4] = {{0,120,180,0},{0,120,180,0},{0,120,180,120}} ;// N = #devices, M = 4 : 50 (min) to 255 (max)

const long DEFAULT_TIME = 1357041600; // Jan 1 2013 00:00 - in seconds
long int HR_SEC = 3600;
long int SEC_MS = 1000;
long int MIN_MS = 5000;
int delayMS = 2000; //milliseconds
long int print_delay = 200;
int level = 0;
float dimMin = 0.1;
float dimMax = 0.8;
float maxPWM = 255;

void chmbloop(int i, int j);

void setup() {
  Serial.begin(9600);
  for (int i = 0; i<Ncmb; i++){
    //setup power pins and set to initial values
    pinMode(powerPin[i],OUTPUT);
    digitalWrite(powerPin[i],HIGH); // some hardware problem, doesn't produce expected response
    for (int j =0; j<Nchn;j++){
          //setup channel relay pins set to initial values
          pinMode(relayPin[i][j],OUTPUT);
          digitalWrite(relayPin[i][j],LOW);
          //setup dimming pins
          pinMode(dimPin[i][j],OUTPUT);
    }
    set_current_stage(0);
    //set_current_stage(1);
  }
}

void loop() {
  // main loop
  sensor_reading();
}

void sensor_reading() {
  if (Serial.available()) {
    String rbpiStr = Serial.readString();
    rbpiStr.replace("\n","");
    DynamicJsonDocument rbpiJson(300);
    deserializeJson(rbpiJson,rbpiStr);

    long int timeserial = rbpiJson["timeserial"];
    syncTime(timeserial);

    Serial.print("{\"message_type\":\"sensor_reading\",\"jbid\":\"lights\",\"readings\":");
    delay(print_delay);
    reading_allsensors();
    Serial.print("}");
    Serial.println();
    Serial.flush();
  }
  dimmer_loop();
}

void dimmer_loop() {
  refresh_stage_status(0);
  //refresh_stage_status(1);
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

  reading_light_status(0);  // read light status cmbid = 0 Left
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

float get_chn_value(int cmbid, int chn) {
  int stg_id = stg_status[cmbid];
  float chn_digital = dim_lvl[chn][stg_id];
  float chn_value = chn_digital/maxPWM;
  return chn_value;
}

void reading_light_status(int cmbid) {
  int j = 0;
  String readingStr;
  for (j;j<Nchn;j++){
    // set the sensor id
    String sensorid = "LED";
    sensorid += String(cmbid);
    sensorid += String(j+1);
    // read the sensor value
    float value = get_chn_value(cmbid,j);

    // create the json reading str
    if (j==0) {
      readingStr = sensor_reading(sensorid,value);
    } else {
      readingStr += sensor_reading(sensorid,value);
    }
    if (j<(Nchn-1)) {
      readingStr += ",";
    }
  }
  Serial.print(readingStr);
  delay(print_delay);
}

void refresh_stage_status(int cmbid) {
  int cur_stg = stg_status[cmbid];
  int action = triggers[cmbid][cur_stg];
  int elapsed = counter[cmbid][cur_stg];
  int remain = action-elapsed;
  if (remain<=0) {
    move_next_stage(cmbid);
    counter[cmbid][cur_stg] = 0;
    SEC_COUNTER = 0;
  }else {
    if (SEC_COUNTER==HR_SEC) {
      counter[cmbid][cur_stg] = counter[cmbid][cur_stg] + 1;
      SEC_COUNTER = 0;
    } else {
      SEC_COUNTER = SEC_COUNTER + 1;
    }
  }
  delay(SEC_MS);
}

void set_current_stage(int cmbid){
  int cur_stg = stg_status[cmbid];
  for (int i = 0; i<Nchn;i++) {
    set_dim_lvl(cmbid,i,dim_lvl[i][cur_stg]);
  }
}

void move_next_stage(int cmbid){
  int cur_stg = stg_status[cmbid];
  int stgid = cur_stg;
  if (cur_stg<3) {
    stgid = cur_stg + 1;
    set_cmb_status(cmbid,1);
  } else {
    stgid = 0;
    set_cmb_status(cmbid,0);
  }
  for (int i = 0; i<Nchn;i++) {
    set_dim_lvl(cmbid,i,dim_lvl[i][stgid]);
  }
  stg_status[cmbid] = stgid;
}

void set_dim_lvl(int cmbid, int chn, int lvl) {
    // determine if ON/OFF
    if (lvl==0) {
      set_cmb_status(cmbid,0);
      set_chn_status(cmbid,chn,0);
    } else {
      set_cmb_status(cmbid,1);
      set_chn_status(cmbid,chn,1);
      set_chn_lvl(cmbid,chn,lvl);
    }
}

void set_cmb_status(int cmbid, int on_status) {
    // turn on the chamber
    digitalWrite(powerPin[cmbid],on_status);
}

void set_chn_status(int cmbid, int chn, int on_status) {
    // turn on the channel relay
    digitalWrite(relayPin[cmbid][chn],on_status);
}

void set_chn_lvl(int cmbid, int chn, int lvl) {
    // set the dim level
    analogWrite(dimPin[cmbid][chn],lvl);
}