import RPi.GPIO as IO       #calling header file which helps us use GPIO’s of PI
import time                 #calling time to provide delays in program
pins = [13,19,26] ; p = {}
frequency = 100
sleepTime = 0.5 #seconds
duty0 = 0
testMax = 100
IO.setwarnings(False)       #do not show any warnings
IO.setmode (IO.BCM)         #we are programming the GPIO by BCM pin numbers. (PIN35 as ‘GPIO19’)
for PinId in pins:
    IO.setup(PinId,IO.OUT)         # initialize GPIO19 as an output.
    p[PinId]=IO.PWM(PinId,frequency)          #GPIO19 as PWM output, with 100Hz frequency
    p[PinId].start(duty0)              #generate PWM signal with 0% duty cycle
while 1:                    #execute loop forever
    for PinId in pins:
        for x in range (testMax):        #execute loop for 50 times, x being incremented from 0 to 49.
            p[PinId].ChangeDutyCycle(x)    #change duty cycle for varying the brightness of LED.
            time.sleep(sleepTime)         #sleep for 100m second
          
        for x in range (testMax):        #execute loop for 50 times, x being incremented from 0 to 49.
            p[PinId].ChangeDutyCycle(testMax-x) #change duty cycle for changing the brightness of LED.
            time.sleep(sleepTime)         #sleep for 100m second