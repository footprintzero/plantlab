import main_controller as mc
import datetime as dt
import pandas as pd

TS_FORMAT = '%Y-%m-%d %H:%M'
ts_filename = 'readings_table.csv'
rcds = []
tsdata = []
ts = None
rd_table = None

def set_time_series():
	global ts
	ts = pd.read_csv(mc.readings_file,parse_dates=['datetime'],dayfirst=True)
	ts.set_index('datetime',inplace=True)

def set_readings_table(min_interval=5):
	global ts, TS_FORMAT, rd_table
	set_time_series()
	dates = list(set([d.date() for d in ts.index]))
	dates.sort()
	hr_str = ['%02d' % h for h in range(23)]
	min_str = ['%02d' % m for m in range(0,60,min_interval)]
	pts = ['%s %s:%s' %(d,h,m) for d in dates for h in hr_str for m in min_str]
	pts = [p for p in pts if p in ts.index]
	intervals = [(pts[i-1],pts[i]) for i in range(1,len(pts))]
	rcds = [ts.loc[iv[0]:iv[1]].groupby('sensorid').mean()['value'] 
			for iv in intervals]
	rd_table = pd.DataFrame.from_records(rcds)
	rd_table['datetime'] = [dt.datetime.strptime(p,TS_FORMAT) for p in pts[1:]]
	rd_table.set_index('datetime',inplace=True)
	rd_table.dropna(how='all')
	

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
