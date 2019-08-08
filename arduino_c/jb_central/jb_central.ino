long int fan_pins[2][2] = {{3,2},{13,12}}; // chamber x wall 2D : {right chmb {right,back},left chmb {left,back}}}
long int man_pins[2] = {7,8}; // right, left chamber

void setup() {
  set_fans();
  //set_manifold();
}


void loop() { 

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