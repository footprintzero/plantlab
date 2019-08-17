import serialcom as src
import pandas as pd
import datetime as dt
import time
import RPi.GPIO as GPIO

#constants
TIME_FORMAT = '%d/%m/%Y %H:%M:%S'
SAMPLING_SEC = 600
RESET_TIMER_MIN = 60
RESET_DELAY_SEC = 30
RELAY_PIN = 17
PIN_ON = 1

#global variables
warnings = []
json_msgs = []
timeserial = None
readings_file = 'sensor_readings.csv'
readings_data = None
fields = ['datetime','sensorid','value']
N_loops = int((RESET_TIMER_MIN*60-2*RESET_DELAY_SEC)/SAMPLING_SEC)

def runloop():
	global json_msgs, warnings
	refresh = True
	json_msgs = []
	setup_relay()
	while 1:
		n = 0
		#loop through each subsystem and get sensor readings
		while (len(warnings)==0 and n<=N_loops):
			get_json_messages(refresh)
			#	syncronize the time
			#	broadcast setpoint instructions
			# 	read warnings and messages

			update_databases()
			# 	read sensor data, read control states
			# 	record data into csv file

			refresh = False
			time.sleep(SAMPLING_SEC)
			n = n + 1
		#reset sensors relay
		reset_relay()


def setup_relay():
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(RELAY_PIN,GPIO.OUT)
	GPIO.output(RELAY_PIN,PIN_ON)


def reset_relay():
	GPIO.output(RELAY_PIN,1-PIN_ON)
	time.sleep(RESET_DELAY_SEC)
	GPIO.output(RELAY_PIN,PIN_ON)
	time.sleep(RESET_DELAY_SEC)


def valid_datestring(dateStr):
	success = 1
	try:
		dtvalue = dt.datetime.strptime(dateStr,TIME_FORMAT)
	except:
		success = 0
	return success

def update_databases():
	global json_msgs
	if len(json_msgs)>0:
		for msg in json_msgs:
			#read sensor data
			if 'readings' in msg:
				update_readings(msg['readings'])

			#read control state ( to be added later..)
	json_msgs = []

def update_readings(json_readings):
	global readings_file, readings_data
	#load readings database
	readings_data = pd.read_csv(readings_file)
	#update database with new records
	records = pd.DataFrame.from_records(json_readings)
	records['valid'] = records.apply(lambda x:valid_datestring(x.datetime),axis=1)
	records.drop(records[records.valid==0].index,inplace=True)
	del records['valid']
	if (len(records)>0) and (set(fields)==set(records.columns)):
		readings_data = readings_data.append(records,ignore_index=True)
		#record data into csv file	
		readings_data.to_csv(readings_file,index=False)

def get_json_messages(refresh):
	global timeserial, json_msgs

	#use serial communication to get json message
	#broadcast setpoint instructions
	#syncronize the time
	src.get_port_msg(refresh)
	timeserial = src.timeserial

	#read warnings and messages

	#get the json message
	if src.success:
		for jsonObj in src.jsons:
			json_msgs.append(jsonObj)
	else:
		print('%s failed to fetch json' %dt.datetime.strftime(dt.datetime.now(),TIME_FORMAT))

if __name__ == "__main__":
	runloop()