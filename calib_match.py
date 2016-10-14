#!/usr/bin/env python

import numpy as np
import matplotlib
from matplotlib import rc, gridspec, cm
import matplotlib.dates as mdates
import matplotlib.pylab as plt
from datetime import date, datetime, timedelta
import os  

def diff_dates(date1, date2):  
    return float(abs(date2-date1).total_seconds() / 60.)

rc('font', **{'family': 'serif', 'serif': ['Computer Modern']})
rc('text', usetex=True)

try:
    os.remove("caliblist.txt")
except OSError:
    pass

f=open("caliblist.txt", "a")

for fn in os.listdir("data/"):
    if fn == 'calibration':
        continue
    datafile = os.path.splitext(fn)[0]    
    datatime = datetime.strptime(datafile[20:40], '%Y-%m-%d.%H_%M_%S')
    calib = ''
    mintime = 10000
    for clb in os.listdir("data/calibration/"):
        calibfile = os.path.splitext(clb)[0]
        calibtime = datetime.strptime(calibfile[21:40], '%Y-%m-%d.%H_%M_%S')
        if abs(diff_dates(datatime, calibtime)) < mintime:
            mintime = abs(diff_dates(datatime, calibtime))
            calib = clb
    f.write('data/' + fn + ',' + 'data/calibration/' + calib + ',' + str(mintime) + '\n')
f.close()
