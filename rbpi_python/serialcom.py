import serial
import time
import os.path
import datetime as dt
import json

MAX_RETRY = 5
wait_time = 2 #seconds
timeserial = 0
timeref = dt.datetime(1970,1,1)
ports = ['/dev/ttyACM0','/dev/ttyACM1','/dev/ttyACM2']
N_ports = 0
srs = []
jsons = [];
msgStr = '';
success = 0;

def get_port_msg(refresh=True):
	global jsons
	if refresh:
		refresh_ports()
	jsons = []
	for s in srs:
		flush_port(s)
		set_current_time()
		msg = fetch_json_message(s)
		jsons.append(msg)

def flush_port(s):
	s.flush()

def refresh_ports():
	global ports, srs, N_ports
	srs = []
	srs = [serial.Serial(p) for p in ports if os.path.exists(p)]
	N_ports = len(srs)

def close_port(s):
	s.close()

def set_current_time():
	global timeserial
	nowtime = dt.datetime.now()
	timeserial= int((nowtime-timeref).total_seconds())

def send_sync_request(s):
	global timeserial
	syncMsg = '{"timeserial":%d}' %timeserial
	s.write(syncMsg)

def fetch_json_message(s):
	global success
	jsonObj = ''
	attemptCount = 0; success = 0
	#get json string from serial port :
	#initiate communication by sending timeserial
	time.sleep(wait_time)
	send_sync_request(s)
	time.sleep(wait_time)
	while ((s.inWaiting()>0) & (attemptCount<MAX_RETRY)):
		msgStr = s.readline().replace('\r\n','')
		foundJson = ((msgStr[0]=='{') & (msgStr[-1]=='}'))
		if foundJson:
			#deserialize json string into json obj
			try:
				jsonObj = json.loads(msgStr)
				success = 1
				break
			except:
				print('error parsing json:%s' %msgStr)
				foundJson=False
				attemptCount = attemptCount+1
		else:
			attemptCount = attemptCount+1
		time.sleep(wait_time)
	if attemptCount>=MAX_RETRY:
		print('max attempts timeout')
	return jsonObj