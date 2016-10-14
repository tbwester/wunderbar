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
   
data = np.genfromtxt("bginfo.txt", dtype=None, delimiter="\t", skip_header=1)

#Fill arrays
time = []
bgmean = [[], [], []]
bgrms = [[], [], []]

for i in range(0, len(data)):
    time.append(datetime.strptime(data[i][0], '%Y-%m-%d.%H:%M:%S'))
    bgmean[0].append(data[i][1])
    bgrms[0].append(data[i][2])
    bgmean[1].append(data[i][3])
    bgrms[1].append(data[i][4])
    bgmean[2].append(data[i][5])
    bgrms[2].append(data[i][6])

bgmean = np.asarray(bgmean)
bgrms = np.asarray(bgrms)

x = time

#Create histograms and plots
fig = plt.figure()
ax = fig.add_subplot(111)

color=iter(cm.rainbow(np.linspace(0,1,len(bgmean))))
for i in reversed(range(0, len(bgmean))):
    c = next(color)
    lbl = str(10**(i + 1)) + " samples"
    ax.errorbar(x, bgmean[i], bgrms[i], fmt='o', color=c, label=lbl)
    
ax.set_title('Bar 1/400 Background Mean \& RMS')
ax.set_xlabel('Time')
deltax = timedelta(minutes=diff_dates(min(x), max(x)) * 0.05)
ax.set_xlim(min(x) - deltax, max(x) + deltax)

ax.set_ylabel('Mean (V)')
ax.legend(loc="best")
myFmt = mdates.DateFormatter('%m-%d\n%H:%M')
ax.xaxis.set_major_formatter(myFmt)

plt.tight_layout()
plt.show()
filename = 'bg.pdf'
plt.savefig(filename)
