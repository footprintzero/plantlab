from sqlalchemy import create_engine
import pandas as pd
import os, sys, shutil

NULL_VALUES = [-1,-127]

INPUT_FOLDER = 'input\\'
OUTPUT_FOLDER = 'reports\\'
SQL_DBNAME = 'sqlite:///plantlab.db'

sensor_reading_tables = {}
engine = None
DATETIME_FIELD = 'datetime'

def load():
    global sensor_reading_tables, engine
    fnames = get_dir_content(INPUT_FOLDER)
    sensrd_filepaths = [INPUT_FOLDER+x for x in fnames if 'sensor_readings_' in x]
    sensrd_names = [x.replace('sensor_readings_','').replace('.csv','')
                    for x in fnames if 'sensor_readings_' in x]
    if len(sensrd_filepaths)>0:
        sensor_reading_tables = dict(zip(sensrd_names,[sensrd_table(x) for x in sensrd_filepaths]))

    #create the SQL instance
    engine = create_engine(SQL_DBNAME, echo=False)

def import_sensor_records():
    load()
    rcd_count= update_sensor_records()
    remove_files('sensor_readings_')
    return rcd_count

def update_sensor_records():
    global engine
    rcd_count = 0
    for tbl_name in sensor_reading_tables:
        tbl = sensor_reading_tables[tbl_name]
        if len(tbl)>0:
            rcd_count+=len(tbl)
            tbl.to_sql('sensor_readings',con=engine,index=False,if_exists='append')
    return rcd_count

def remove_files(tag='sensor_readings_'):
    fnames = get_dir_content(INPUT_FOLDER)
    filepaths = [INPUT_FOLDER+x for x in fnames if tag in x]
    for f in filepaths:
        os.remove(f)

def sensrd_table(filepath):
    df = pd.read_csv(filepath,parse_dates=[DATETIME_FIELD],dayfirst=True)
    df = cleanup(df)
    return df

def cleanup(df):
    #drop na
    df.dropna(inplace=True)
    if len(df)>0:
        df = df[~df.value.isin(NULL_VALUES)].copy()
    return df

def get_dir_content(dest,infotype='file'):
    if infotype=='file':
        dir_content = [f for f in os.listdir(dest)
                     if os.path.isfile(os.path.join(dest,f))]
    elif infotype=='dir':
        dir_content = [f for f in os.listdir(dest)
                    if os.path.isdir(os.path.join(dest, f))]
    return dir_content


if __name__ == "__main__":
    if len(sys.argv)>1:
        command_str = sys.argv[1]
        if command_str == 'import_sensor_records':
            rcd_count = import_sensor_records()
            if rcd_count>0:
                print('%s new records imported' % rcd_count)
            else:
                print('no new records imported')
