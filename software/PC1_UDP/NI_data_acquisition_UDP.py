#!/usr/bin/python
# -*-coding: utf-8 -*-
# author: Zenan-SSR
# data: 2022年10月7日
# description: 读取 NI-cDAQ_9174基于NI_9220采集卡的四路TENG数据并实时显示波形.(滤波、UDP发送处理数据)
# project: Thumb_sight项目

import nidaqmx
from nidaqmx.constants import Edge
from nidaqmx.constants import AcquisitionType
import numpy as np
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtGui
import datetime
import time
import csv
import socket

Thumb_sight_raw_data = np.zeros(5)
Thumb_sight_buffer = np.zeros([5, 40])

# 全局变量用于NI_daq数据保存
str_time = datetime.datetime.now().strftime('%Y%m%d%H%M%S')
f_DAQ = open('Thumb_sight_raw_data_' + str_time + '.csv', 'w', encoding='utf-8', newline='')
header1 = [str_time, '4CH_Thumb_sight_raw_data']
header2 = ['SysTime(ms)', 'V_CH1', 'V_CH2', 'V_CH3', 'V_CH4']
writer_UDP = csv.writer(f_DAQ)
writer_UDP.writerow(header1)
writer_UDP.writerow(header2)

# 绘图相关
win = pg.GraphicsLayoutWidget(show=True, title='Thumb sight raw data')
# win.setWindowTitle('pyqtgraph example: Scrolling Plots')

# raw-data
p1 = win.addPlot(row=0, col=0)
p1.setYRange(-0.25, 0.25)
p1.showGrid(x=True, y=True)
p1.setLabel("left", "V_ch1")
data1 = 0*np.ones(200)
curve1 = p1.plot(data1, pen=(1, 4))

p2 = win.addPlot(row=1, col=0)
p2.setYRange(-0.25, 0.25)
p2.showGrid(x=True, y=True)
p2.setLabel("left", "V_ch2")
data2 = 0*np.ones(200)
curve2 = p2.plot(data2, pen=(2, 4))

p3 = win.addPlot(row=2, col=0)
p3.setYRange(-0.25, 0.25)
p3.showGrid(x=True, y=True)
p3.setLabel("left", "V_ch3")
data3 = 0*np.ones(200)
curve3 = p3.plot(data3, pen=(3, 4))

p4 = win.addPlot(row=3, col=0)
p4.setYRange(-0.25, 0.25)
p4.showGrid(x=True, y=True)
p4.setLabel("left", "V_ch4")
data4 = 0*np.ones(200)
curve4 = p4.plot(data4, pen=(4, 4))

V_ch1 = 0
V_ch2 = 0
V_ch3 = 0
V_ch4 = 0

# read 4 channels of the NI_9220 采集卡
def read_raw_data():
    # global task
    # 设置
    with nidaqmx.Task() as task:
        task.ai_channels.add_ai_voltage_chan("cDAQ3Mod2/ai4:7")
        task.timing.cfg_samp_clk_timing(rate=100,
                                        sample_mode=AcquisitionType.FINITE, samps_per_chan=40)  # active_edge=Edge.RISING, , sample_mode=AcquisitionType.FINITE, samps_per_chan=50
        raw_data = np.array(task.read(40))
        # print(raw_data)
    return raw_data
    # with nidaqmx.Task() as task:
    #     task.ai_channels.add_ai_voltage_chan("cDAQ3Mod2/ai4:7")
    #     task.timing.cfg_samp_clk_timing(100, source="")  # active_edge=Edge.RISING, , sample_mode=AcquisitionType.FINITE, samps_per_chan=50
    #     raw_data = np.array(task.read())
    #     # print(raw_data)
    # return raw_data

    # with nidaqmx.Task() as task:
    #     task.ai_channels.add_ai_voltage_chan("cDAQ3Mod2/ai4:7")
    #     raw_data = np.array(task.read())
    # return raw_data

# 画图进程
def update1():
    global data1, curve1, V_ch1, \
        data2, curve2, V_ch2, \
        data3, curve3, V_ch3, \
        data4, curve4, V_ch4, Thumb_sight_buffer

    data1[:-10] = data1[10:]  # shift data in the array one sample left
    # (see also: np.roll)
    data1[-10: -1] = Thumb_sight_buffer[0, -10:-1]
    curve1.setData(data1)

    data2[:-10] = data2[10:]  # shift data in the array one sample left
    # (see also: np.roll)
    data2[-10: -1] = Thumb_sight_buffer[1, -10:-1]
    curve2.setData(data2)

    data3[:-10] = data3[10:]  # shift data in the array one sample left
    # (see also: np.roll)
    data3[-10: -1] = Thumb_sight_buffer[2, -10:-1]
    curve3.setData(data3)

    data4[:-10] = data4[10:]  # shift data in the array one sample left
    # (see also: np.roll)
    data4[-10: -1] = Thumb_sight_buffer[3, -10:-1]
    curve4.setData(data4)


# UDP数据采集进程
def update2():
    global V_ch1, V_ch2, V_ch3, V_ch4, writer_UDP, Thumb_sight_raw_data, Thumb_sight_buffer
    t = time.time()
    Thumb_sight_buffer = read_raw_data()
    Thumb_sight_raw_data[0] = t*1000
    Thumb_sight_raw_data[1:5] = Thumb_sight_buffer[:, -1]
    writer_UDP.writerow(Thumb_sight_raw_data)
    V_ch1 = Thumb_sight_raw_data[1]
    V_ch2 = Thumb_sight_raw_data[2]
    V_ch3 = Thumb_sight_raw_data[3]
    V_ch4 = Thumb_sight_raw_data[4]

    # #  创建一个udp套接字
    # udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    # #   发送数据（数据，(ip,端口)）
    # udp_socket.sendto(Thumb_sight_raw_data, ("10.168.1.127", 12333))
    # #  关闭套接字
    # udp_socket.close()

    print(Thumb_sight_raw_data)


timer1 = pg.QtCore.QTimer()
timer1.timeout.connect(update1)
timer1.start(5)

timer2 = pg.QtCore.QTimer()
timer2.timeout.connect(update2)
timer2.start(10)


if __name__ == '__main__':
    # system = nidaqmx.system.System.local()
    # system.driver_version
    # for device in system.devices:
    #     print(device)

    # while 1:
    #     Thumb_sight_raw_data = read_raw_data()
    #     print(Thumb_sight_raw_data)
    pg.exec()
