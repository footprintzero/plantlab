'''Created on 2018/11/17
   last update 2018/12/07
@author: Taylor W Hickem
'''
import sys
import RPi.GPIO as GPIO
import datetime as dt
import logger

LOGGER_STATUS = True    #log results to csv file Y/N?

#define the settings
light_settings = {}
pump_settings = {}

light_settings['start_hrs'] = 6
light_settings['lights-T'] = {'OFF hrs':8.00}
light_settings['lights-M'] = None
light_settings['lights-L'] = {'OFF hrs':8.00}

pump_settings['pumpA'] = {'ON sec':30,'OFF sec':900,'last_update':None,'last_value':None}
pump_settings['pumpB'] = {'ON sec':60,'OFF sec':600,'last_update':None,'last_value':None}

#define the GPIO pin arrangement
pins = {'pumpA':17,'pumpB':5,'lights-M':25,'lights-L':23,'lights-T':24}

#define the default value settings
# the pumps are configured as NC and the relay is inverted
# so a 1 corresponds to the OFF position
# for the lights, they are configured as NO so a 1 corresponds to ON
ON_values = {'pumpA':0,'pumpB':0,'lights-M':1,'lights-L':1,'lights-T':1}
defaultValues = {'pumpA':False,'pumpB':False,'lights-T':True,'lights-M':False,'lights-L':True}

def setupPins():
    for pin in pins:
        GPIO.setup(pins[pin],GPIO.OUT)

def pinIsON(pin_key):
    pinId = pins[pin_key]
    pinONValue = ON_values[pin_key]
    GPIOValue = GPIO.input(pinId)
    isON = (GPIOValue==pinONValue)
    return isON

def setPinValue(pin_key,isON):
    pinId = pins[pin_key]
    pinONValue = ON_values[pin_key]
    if isON:
        outputValue = pinONValue
    else:
        outputValue = 1-pinONValue
    GPIO.output(pinId,outputValue)
    
def set_defaultValues():
    for pin in pins:
        setPinValue(pin,defaultValues[pin])
        if LOGGER_STATUS:
            logger.writelog_controller_action('set default value'
                ,defaultValues[pin]*1,'set default value for %s' % pin)
        
def updateLightStatus(light_key,nowTime):
    settings = light_settings[light_key]
    if not settings is None:
        start_hrs = light_settings['start_hrs']
        offHrs = settings['OFF hrs']
        onHrs = 24 - offHrs
        isON = pinIsON(light_key)
        if start_hrs+onHrs < 24: # OFF time is within same day
            sameDay = True
            end_hrs = start_hrs + onHrs
        else:   #OFF time is the next day
            sameDay = False
            end_hrs = onHrs - (24-start_hrs)
        now_hrs = nowTime.hour + float(nowTime.minute)/60
        if sameDay:
            elapsed = [now_hrs-start_hrs,now_hrs-end_hrs]
            if elapsed[0]<0 and elapsed[1]<0: # OFF
                target_state = False
            elif elapsed[0]>=0 and elapsed[1]<0: # ON
                target_state = True
            else: # OFF
                target_state = False
        else:
            elapsed = [now_hrs-end_hrs,now_hrs-start_hrs]
            if elapsed[0]<0 and elapsed[1]<0: # ON
                target_state = True
            elif elapsed[0]>=0 and elapsed[1]<0: # OFF
                target_state = False
            else: # ON
                target_state = True
        if not (isON == target_state):
            setPinValue(light_key,target_state)
            if LOGGER_STATUS:
                logger.writelog_controller_action('change light status'
                    ,target_state*1,'changed light status for %s' % light_key)
            

def updatePumpStatus(pump_key,nowTime):
    
    def changePumpStatus(isON):
        setPinValue(pump_key,isON)
        pump_settings[pump_key]['last_update'] = nowTime
        pump_settings[pump_key]['last_value'] = isON
        if LOGGER_STATUS:
            logger.writelog_controller_action('change pump status'
                ,isON*1,'changed pump status for %s' % pump_key)

    settings = pump_settings[pump_key]
    last_update = settings['last_update']
    last_value = settings['last_value']
    isON = pinIsON(pump_key)
    if last_update is None:
        last_update = nowTime
        last_value = isON
        pump_settings[pump_key]['last_update'] = last_update
        pump_settings[pump_key]['last_value'] = last_value
    ONOFF_sec = (settings['ON sec'],settings['OFF sec'])
    elapsed = nowTime - last_update
    if last_value:  #ON
        if elapsed.seconds>ONOFF_sec[0]:
            if isON:
                changePumpStatus(False)
        else:
            if not isON:
                changePumpStatus(True)
    else:           #OFF
        if elapsed.seconds>ONOFF_sec[1]:
            if not isON:
                changePumpStatus(True)
        else:
            if isON:
                changePumpStatus(False)

def updateLightsStatus(nowTime):
    for light_key in [x for x in light_settings if not x == 'start_hrs']:
        updateLightStatus(light_key,nowTime)

def updatePumpsStatus(nowTime):
    for pump_key in pump_settings:
        updatePumpStatus(pump_key,nowTime)
    
def run_process():
    while True:
        nowTime = dt.datetime.now()
        updateLightsStatus(nowTime)
        updatePumpsStatus(nowTime)

def main():
        # set the mode
        GPIO.setmode(GPIO.BCM)

        # setup the pins
        setupPins()
        
        # set pins to default values
        set_defaultValues()
        
        # run the process
        run_process()


if __name__ == "__main__":
    try:
        main()
    except BaseException as e:
        print('shutting down for exception %s' % e)
        print(e.message)
        if LOGGER_STATUS:
            logger.writelog_exception(
                str(e.message),
                value=str(e.args),
                exception_type=type(e).__name__)
        # set to default values
        set_defaultValues()
        GPIO.cleanup()
        sys.exit(0)