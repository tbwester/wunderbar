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
   
data = np.genfromtxt("fitinfo.txt", dtype=None, delimiter="\t", skip_header=1)
data2 = np.genfromtxt("fitinfo2.txt", dtype=None, delimiter="\t", skip_header=1)
cdata = np.genfromtxt("caliblist.txt", dtype=None, delimiter=",", skip_header=0)

#Fill array 1 (sipm cut)
time = []
mean = []
meanerr = []

for i in range(0, len(data)):
    time.append(datetime.strptime(data[i][0], '%Y-%m-%d.%H:%M:%S'))
    mean.append(data[i][1])
    meanerr.append(data[i][2])

x = time
y = mean
dy = meanerr

#Fill array 2 (background subtraction)
time2 = []
mean2 = []
meanerr2 = []

for i in range(0, len(data2)):
    time2.append(datetime.strptime(data2[i][0], '%Y-%m-%d.%H:%M:%S'))
    mean2.append(data2[i][1])
    meanerr2.append(data2[i][2])

cdiff = []
for i in range(0, len(cdata)):
    cdiff.append(cdata[i][2])    

x2 = time2
y2 = mean2
dy2 = meanerr2

#differr = np.sqrt(np.asarray(dy2)**2 + np.asarray(dy)**2)

#Create histograms and plots
fig = plt.figure()
gs = gridspec.GridSpec(2, 1, height_ratios=[3, 1])
ax = fig.add_subplot(111)#gs[0])
#ax2= fig.add_subplot(gs[1])

ax.errorbar(x, y, dy, fmt='o', color='b', label='SiPM Cut')
ax.errorbar(x2, y2, dy2, fmt='o', color='g', label='Background Subtract')

#ax2.errorbar(cdiff, np.abs(np.asarray(y) - np.asarray(y2)), differr, fmt='o', color='r', label='Data/Calib Time Diff.')

ax.set_title('Bar 1/400 Voltage Peak Stability')
#ax.set_xlabel('Time')
deltax = timedelta(minutes=diff_dates(min(x), max(x)) * 0.05)
ax.set_xlim(min(x) - deltax, max(x) + deltax)
#ax2.set_xlim(min(x) - deltax, max(x) + deltax)

ax.set_ylabel('Peak (V)')
ax.legend(loc="best")

#ax2.set_ylabel('Fit Diff (V)')
#ax2.set_xlabel('Data/Calib Time Diff. (min)')
#ax2.legend(loc="best")

#ax.set_xticklabels([])
myFmt = mdates.DateFormatter('%m-%d\n%H:%M')
ax.xaxis.set_major_formatter(myFmt)


plt.tight_layout()
plt.show()
filename = 'stability.pdf'
plt.savefig(filename)
