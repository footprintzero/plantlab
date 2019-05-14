import main_controller as mc
import datetime as dt
import pandas as pd

TS_FORMAT = '%Y-%m-%d %H:%M'
ts_filename = 'readings_table.csv'
rcds = []
tsdata = []

def refresh_tsdata(recent_hrs=16,fetchall=False):
	global tsdata, ts_filename, rcds
	# import file	datetime, sensorid, value
	rcds = pd.read_csv(mc.readings_file,parse_dates=['datetime'],dayfirst=True)
	rcds.dropna(subset=['datetime'],inplace=True)
	if not fetchall:
		rcds.set_index('datetime',inplace=True)
		delta= dt.timedelta(hours=recent_hrs)
		nowtime = dt.datetime.now()
		starttime = dt.datetime.strftime(nowtime-delta,TS_FORMAT)
		endtime = dt.datetime.strftime(nowtime,TS_FORMAT)
		rcds = rcds[starttime:endtime]
		rcds.reset_index(inplace=True)
	rcds['dt_short'] = rcds.apply(lambda x:dt.datetime(year=x.datetime.year,month=x.datetime.month,
		day=x.datetime.day,hour=x.datetime.hour,minute=x.datetime.minute),axis=1)
	del rcds['datetime']
	rcds.rename(columns={'dt_short':'datetime'},inplace=True)
	

	#create pivot table
	#		datetime, value 
	tsdata = pd.pivot_table(rcds,values='value',columns='sensorid',index='datetime')
	tsdata.to_csv(ts_filename,index=False)

def show_recent(rcd_limit=20):
	global tsdata
	refresh_tsdata()
	print(tsdata.tail(rcd_limit))

#optional method 2 : divided into individual time series for each sensor	
#		datetime, value 
#sensorids = list(rcds.sensorid.unique())
#sen_srs = [rcds[rcds.sensorid==x][['datetime','value']].
#		set_index('datetime').rename(columns={'value':x}).copy() for x in sensorids]
