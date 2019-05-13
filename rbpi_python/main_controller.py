import serialcom as src
import pandas as pd
import datetime as dt
import time

#constants
TIME_FORMAT = '%d/%m/%Y %H:%M:%S'
SAMPLING_SEC = 180

#global variables
warnings = []
json_msgs = []
timeserial = None
readings_file = 'sensor_readings.csv'
readings_data = None

def runloop():
	global json_msgs, warnings
	refresh = True
	json_msgs = []
	#loop through each subsystem
	while (len(warnings)==0):
		get_json_messages(refresh)
		#	syncronize the time
		#	broadcast setpoint instructions
		# 	read warnings and messages	

		update_databases()
		# 	read sensor data, read control states
		# 	record data into csv file

		refresh = False
		time.sleep(SAMPLING_SEC)

def update_databases():
	global json_msgs
	if len(json_msgs)>0:
		for msg in json_msgs:
			#read sensor data
			if 'readings' in msg:
				update_readings(msg['readings'])

			#read control state ( to be added later..)

def update_readings(json_readings):
	global readings_file, readings_data
	#load readings database
	readings_data = pd.read_csv(readings_file)
	#update database with new records
	records = pd.DataFrame.from_records(json_readings)
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
		json_msgs.append(src.jsonObj)

if __name__ == "__main__":
	runloop()