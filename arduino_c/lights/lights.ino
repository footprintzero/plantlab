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

// declare variables and functions
int Ncmb = 2;
int Nchn = 3;
int powerPin[] = {0,1};
int relayPin[][3] = {{13,12,8},{7,4,2}};
int dimPin[][3] = {{11,10,9},{6,5,3}};
int relayValue =LOW;

int devCount = 1;
long int dev_ids[6][2] = {{0,0},{0,1},{0,2},{1,0},{1,1},{1,2}};
long int dev_status[3] = {1,1,1}; // 
long int counter[3][2] = {{4,0},{4,0},{4,0}}; // N = #devices, M = 2 : ON , OFF
long int triggers[3][2] = {{12,12},{12,12},{12,12}} ;// N = #devices, M = 2 : ON time (hrs), OFF time (hrs)
long int dim_lvl[2][3] = {{120,120,120},{120,120,120}} ;// N = #devices, M = 2 : 50 (min) to 255 (max)

long int HR_MS = 3600000;
int delayMS = 2000; //milliseconds
int level = 0;
float dimMin = 0.1;
float dimMax = 0.8;
int maxPWM = 255;

void chmbloop(int i, int j);

void setup() {
  for (int i = 0; i<Ncmb; i++){
    //setup power pins and set to initial values
    pinMode(powerPin[i],OUTPUT);
    digitalWrite(powerPin[i],HIGH);
    for (int j =0; j<Nchn;j++){
          //setup channel relay pins set to initial values
          pinMode(relayPin[i][j],OUTPUT);
          digitalWrite(relayPin[i][j],LOW);
          //setup dimming pins
          pinMode(dimPin[i][j],OUTPUT);
    }  
  }
  toggle_lights(dev_ids[0],dev_status[0]);
  toggle_lights(dev_ids[1],dev_status[0]);
  toggle_lights(dev_ids[2],dev_status[0]);
  //for (int i = 0; i<Ncmb; i++){
  //  chmbloop(i);
  //}
  //geometry_test();
}

void timer_mainloop() {
  // deviceid is ambiguous, call all devices
  for (int i = 0; i<devCount;i++) {
    // call ambiguously for both ON and OFF triggers
    timer_deviceloop(i);
  }
  delay(HR_MS);
  //delay(SEC_MS);  
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
      toggle_lights(dev_ids[deviceid],callid);
      toggle_lights(dev_ids[deviceid+1],callid);
      toggle_lights(dev_ids[deviceid+2],callid);
      //digitalWrite(dev_pins[deviceid], callid);      
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

void loop() {
  // main loop
  // for (int i = 0; i<Ncmb; i++){
  //  chmbloop(i);
  //}
  timer_mainloop();
  //geometry_test();
  //delay(1000);
}

void chmbloop(int i) {
  //1) first power-on the chamber
  digitalWrite(powerPin[i],LOW);    
  //2) close all relays to turn off the channels 
  close_channels(i);
  //3) dimming loop : for each channel i,j
  //dim_test(i);
  //4) power-OFF the chamber
  // digitalWrite(powerPin[i],HIGH);    
}

void close_channels(int i) {
  for (int j = 0; j<Nchn; j++){
    digitalWrite(relayPin[i][j],LOW);    
  }  
}
void toggle_lights(long int tp[],int callid) {
  // define the channel
  //int tp[] = {0,0};
  int geotest_lvl = dim_lvl[tp[0]][tp[1]];
  //int geotest_lvl = 255;
  if (callid==1) {
    // turn on the chamber
    digitalWrite(powerPin[tp[0]],LOW);
    // turn on the channel relay
    digitalWrite(relayPin[tp[0]][tp[1]],HIGH);
    // set the dim level
    analogWrite(dimPin[tp[0]][tp[1]],geotest_lvl);  
  } else {
    // turn off the chamber
    digitalWrite(powerPin[tp[0]],HIGH);
    // turn off the channel relay
    digitalWrite(relayPin[tp[0]][tp[1]],LOW);
  }
}

void geometry_test() {
  // define the channel
  int tp[] = {0,0};
  int geotest_lvl = 255;
  // turn on the chamber
  digitalWrite(powerPin[tp[0]],LOW);
  // turn on the channel relay
  digitalWrite(relayPin[tp[0]][tp[1]],HIGH);
  // set the dim level
  analogWrite(dimPin[tp[0]][tp[1]],geotest_lvl);  

  // use this code for all 3x simultaneous
  //digitalWrite(relayPin[tp[0]][1],HIGH);
  //digitalWrite(relayPin[tp[0]][2],HIGH);
  
  // set the dim level to max
  
  // use this code for all 3x simultaneous
  //analogWrite(dimPin[tp[0]][tp[1]],geotest_lvl);  
  //analogWrite(dimPin[tp[0]][1],geotest_lvl);  
  //analogWrite(dimPin[tp[0]][2],geotest_lvl);  

  //delay(1000);
  //analogWrite(dimPin[tp[0]][tp[1]],0);  
  //digitalWrite(relayPin[tp[0]][tp[1]],LOW);
  //digitalWrite(powerPin[tp[0]],HIGH);

  //int tz[] = {1,2};
  //int geotest_z = 255;
  // turn on the chamber
  //digitalWrite(powerPin[tz[0]],LOW);
  // turn on the channel relay
  //digitalWrite(relayPin[tz[0]][tz[1]],HIGH);
  // set the dim level to max
  //analogWrite(dimPin[tz[0]][tz[1]],geotest_z);  

  //delay(1000);
  //analogWrite(dimPin[tz[0]][tz[1]],0);  
  //digitalWrite(relayPin[tz[0]][tz[1]],LOW);
  //digitalWrite(powerPin[tz[0]],HIGH);
    
}


void dim_test(int i) {
  // turn on the relay
  for (int j = 0; j<Nchn; j++){
    digitalWrite(relayPin[i][j],HIGH);
    // then dim from min to max range
    for (float dimValue = dimMin ; dimValue < dimMax; dimValue+=0.1) {
        level = dimValue*maxPWM;
        analogWrite(dimPin[i][j],level);
        // add a delay
        delay(delayMS);
    }
  // then turn off the channel relay
    digitalWrite(relayPin[i][j],LOW);    
  }
}
