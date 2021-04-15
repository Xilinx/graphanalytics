#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#

import sys
import re
from datetime import datetime
import tkinter as tk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from pandas import DataFrame


FIGURE_DPI = 100

log_file = sys.argv[1]

with open(log_file, 'r', encoding='utf-8', errors='ignore') as fp:
    log_lines = fp.readlines()

#CuReport: 140441970509568::udf_cosinesim_ss_fpga req=1616448355459841300 lock=1616448355459841878 release=1616448355462792067
re_cureport = re.compile('CuReport: (\S+)::(\S+) req=(\d+) lock=(\d+) release=(\d+)')
first_cu_line = True
prev_lock_tick_ms = 0
runtime_buckets = {} # bucket for each second
lock_wait_times = []
for line in log_lines:
    m = re_cureport.match(line)
    if m:

        req_tick_ms = int(m.group(3))/1000000
        bucket_sec = req_tick_ms//1000

        lock_tick_ms = int(m.group(4))/1000000
        if first_cu_line:
            idle_time = 0
            first_cu_line = False
        else:
            idle_time = lock_tick_ms - prev_lock_tick_ms

        release_tick_ms = int(m.group(5))/1000000

        req_dt = datetime.fromtimestamp( req_tick_ms / 1000)
        lock_wait_ms = lock_tick_ms - req_tick_ms
        lock_wait_times.append(lock_wait_ms)
        # runtime including lock wait time
        runtime_ms = release_tick_ms - req_tick_ms
        req_time_str = req_dt.strftime("%Y-%m-%d %H:%M:%S.%f")
        print(req_dt, lock_wait_ms, runtime_ms, idle_time)

        if runtime_buckets.get(bucket_sec) is None:
            runtime_buckets[bucket_sec] = runtime_ms
        else:
            runtime_buckets[bucket_sec] += runtime_ms

        prev_lock_tick_ms = lock_tick_ms

time_hist = []
cu_util_hist = []
for k in sorted(runtime_buckets):
    time_hist.append(datetime.fromtimestamp(k).strftime("%Y-%m-%d %H:%M:%S"))
    cu_util_hist.append(runtime_buckets[k]*100/1000)

print('INFO: minimum wait time:', min(lock_wait_times))
print('INFO: average wait time:', sum(lock_wait_times)/len(lock_wait_times))
print('INFO: maximum wait time:', max(lock_wait_times))


root_window = tk.Tk()
root_window.geometry('1500x700+20+20')
root_window.title('CU Utilization Report')
root_window.grid_rowconfigure(0, weight=1)
root_window.grid_columnconfigure(0, weight=1)

cur_grid_row = 0

# plot row
figure_hist = plt.Figure(figsize=(10, 5), dpi=FIGURE_DPI)
plot_hist = figure_hist.add_subplot(111)
canvas_hist = FigureCanvasTkAgg(figure_hist, root_window)
canvas_hist.get_tk_widget().grid(row=cur_grid_row, columnspan=4, sticky='nsew')

cur_grid_row = cur_grid_row + 1

# Plot navigation toolbar
frame_toolbar = tk.Frame(root_window)
frame_toolbar.grid(row=cur_grid_row, columnspan=4)
toolbar_plot = NavigationToolbar2Tk(canvas_hist, frame_toolbar)
cur_grid_row = cur_grid_row + 1


y_hist_dict = {'time': time_hist,
               'cu_util': cu_util_hist}
y_hist_df = DataFrame(y_hist_dict, columns=['time', 'cu_util'])
y_hist_df.plot(kind='line', legend=True, x='time', y='cu_util',
               ax=plot_hist, color='r', marker='.', fontsize=10)
plot_hist.set_ylabel('CU Utilization %')
plot_hist.set_title('CU Utilization (%) History')
canvas_hist.draw()

root_window.mainloop()



