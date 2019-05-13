import serial
import time
import os.path
import datetime as dt
import json

MAX_RETRY = 5
wait_time = 2 #seconds
timeserial = 0
timeref = dt.datetime(1970,1,1)
ports = ['/dev/ttyACM0','/dev/ttyACM1']
s = None
jsonObj = None;
msgStr = '';
success = 0;

def get_port_msg(refresh=True):
	if refresh:
		refresh_ports()
	set_current_time()
	fetch_json_message()

def refresh_ports():
	global ports, s
	srs = []
	srs = [serial.Serial(p) for p in ports if os.path.exists(p)]
	s = srs[0]
	s.flush()

def close_port():
	global s
	s.close()

def set_current_time():
	global timeserial
	nowtime = dt.datetime.now()
	timeserial= int((nowtime-timeref).total_seconds())

def send_sync_request():
	global s, timeserial
	syncMsg = '{"timeserial":%d}' %timeserial
	s.write(syncMsg)

def fetch_json_message():
	global s, jsonObj, success
	attemptCount = 0; success = 0
	#get json string from serial port :
	#initiate communication by sending timeserial
	time.sleep(wait_time)
	send_sync_request()
	time.sleep(wait_time)
	while ((s.inWaiting()>0) & (attemptCount<MAX_RETRY)):
		msgStr = s.readline().replace('\r\n','')
		foundJson = ((msgStr[0]=='{') & (msgStr[-1]=='}'))
		if foundJson:
			#deserialize json string into json obj
			jsonObj = json.loads(msgStr)
			success = 1
			break
		else:
			attemptCount = attemptCount+1
	time.sleep(wait_time)		
	