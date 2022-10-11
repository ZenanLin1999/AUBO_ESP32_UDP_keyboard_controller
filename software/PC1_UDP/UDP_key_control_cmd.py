#!/usr/bin/python
# -*-coding: utf-8 -*-
# author: Zenan-SSR
# data: 2022年10月10日
# description: 按键键值->UDP发送指定code到
# project: AUBO-PC1-UDP 项目

import socket
import numpy as np
import keyboard
import time


Target_IP = "10.168.1.224"
Target_PORT = 23030
current_key = -1
last_key = -1


# ip: "192.168.5.1"
# port: 1234
# send_data: 'Thumb_sight_raw_data\r\n'
def send_to_port(send_data, ip, port):
    #  创建一个udp套接字
    udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    #   发送数据（数据，(ip,端口)）
    udp_socket.sendto(send_data.encode("utf-8"), (ip, port))
    #  关闭套接字
    udp_socket.close()


last_time = time.time()*1000  # 转换为ms

if __name__ == '__main__':
    while True:
        this_time = time.time()*1000  # 转换为ms
        if keyboard.is_pressed("q"):  # 退出
            print("You pressed q")
            break
        elif keyboard.is_pressed("a") and (current_key != 0 or this_time-last_time > 500):  # 0°
            print("AUBO i5 turn 0°")
            send_to_port('AA00BB', Target_IP, Target_PORT)
            current_key = 0
            last_time = this_time
        elif keyboard.is_pressed("s") and (current_key != 1 or this_time-last_time > 500):  # 60°
            print("AUBO i5 turn 60°")
            send_to_port('AA01BB', Target_IP, Target_PORT)
            current_key = 1
            last_time = this_time
        elif keyboard.is_pressed("d") and (current_key != 2 or this_time-last_time > 500):  # 120°
            print("AUBO i5 turn 120°")
            send_to_port('AA02BB', Target_IP, Target_PORT)
            current_key = 2
            last_time = this_time
        elif keyboard.is_pressed("n") and (current_key != 3 or this_time-last_time > 500):  # -z
            print("AUBO i5 turn -z")
            send_to_port('AA03BB', Target_IP, Target_PORT)
            current_key = 3
            last_time = this_time
        elif keyboard.is_pressed("m") and (current_key != 4 or this_time-last_time > 500):  # +z
            print("AUBO i5 turn +z")
            send_to_port('AA04BB', Target_IP, Target_PORT)
            current_key = 4
            last_time = this_time
        elif keyboard.is_pressed("z") and (current_key != 5 or this_time-last_time > 500):  # -r
            print("AUBO i5 turn -r")
            send_to_port('AA05BB', Target_IP, Target_PORT)
            current_key = 5
            last_time = this_time
        elif keyboard.is_pressed("x") and (current_key != 6 or this_time-last_time > 500):  # +r
            print("AUBO i5 turn +r")
            send_to_port('AA06BB', Target_IP, Target_PORT)
            current_key = 6
            last_time = this_time



# import keyboard
#
# while True:
#     if keyboard.read_key() == "p":
#         print("You pressed p")
#         break
#
# while True:
#     if keyboard.is_pressed("q"):
#         print("You pressed q")
#         break

# keyboard.on_press_key("r", lambda _: print("You pressed r"))