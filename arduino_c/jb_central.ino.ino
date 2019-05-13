int fan_rc_rw = 3;
int fan_rc_bw = 2;
int fan_lc_lw = 1;
int fan_lc_bw = 0;
int mf_rc = 7;
int mf_lc = 8;


void setup() {
  
  pinMode(fan_rc_rw, OUTPUT);
  digitalWrite(fan_rc_rw, HIGH);
  
  pinMode(fan_rc_bw, OUTPUT);
  digitalWrite(fan_rc_bw, HIGH);
  
  pinMode(fan_lc_lw, OUTPUT);
  digitalWrite(fan_lc_lw, LOW);
  
  pinMode(fan_lc_bw, OUTPUT);
  digitalWrite(fan_lc_bw, LOW);

  pinMode(mf_rc, OUTPUT);
  digitalWrite(mf_rc, HIGH);

  pinMode(mf_lc, OUTPUT);
  digitalWrite(mf_lc, HIGH);
}


//void loop() {
  //digitalWrite(fan_rc_rw, HIGH); //righ chamber_right wall relay 5
  //delay(1000);
  //digitalWrite(fan_rc_rw, LOW);
  //delay(1000);

  //digitalWrite(fan_rc_bw, HIGH); //righ chamber_back wall relay 6
  //delay(1000);
  //digitalWrite(fan_rc_bw, LOW);
  //delay(1000);

  //digitalWrite(fan_lc_lw, HIGH); //left chamber_left wall relay 7
  //delay(1000);
  //digitalWrite(fan_lc_lw, LOW);
  //delay(1000);

  //digitalWrite(fan_lc_bw, HIGH); //left chamber_left wall relay 8
  //delay(1000);
  //digitalWrite(fan_lc_bw, LOW);
  //delay(1000);
//}

void loop() {
 
}
