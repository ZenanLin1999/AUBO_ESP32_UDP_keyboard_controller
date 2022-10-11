// author: Zenan-SSR
// date: 2022/10/11
// descrition: Using esp32 uart control AUBO movement

#include "TENG_experiment.h"
#include "ATI_Nano43_serial.h"
#include <stdio.h>
#include <unistd.h>
#include <iostream>


// 0x41+0x41+0x30+0x30+0x42+0x42+0x0A
// example:(7 bytes)
// 41 41 30 30 42 42 0A
const uint8_t buff_len = 7;
uint8_t buff[buff_len]; // read buffer
const char *dev  = "/dev/ttyUSB0";

Force_Data ATI_Nano43_data;

int main()
{
    //TENG experiment
    TENG_experiment zenan_demo;

    // basic serial port setup
    serialPort myserial;
    int nread;
    double One_step = 0.02;

    cout<<"SerialPort Open!"<<endl;
    myserial.OpenPort(dev);
    myserial.setup(115200,0,8,1,'N');

    while (true)
    {
      // 00-read a frame.
      nread = myserial.readBuffer(buff, buff_len); //

      // 01-only for debug(print raw data)
      for(int i = 0;i<buff_len;i++)
      {
          printf("%02x ", buff[i]);
      }
      printf("\r\n");

      // 03-decode the command.
      if(buff[0] == 0x41 && buff[1] == 0x41 && buff[4] == 0x42 && buff[5] == 0x42)
      {
          if(buff[2] == 0x30 && buff[3] == 0x30)
          {
            zenan_demo.target_movement(0, One_step);
            printf("AUBO i5 turn 0°\r\n");
          }

          else if(buff[2] == 0x30 && buff[3] == 0x31)
          {
            zenan_demo.target_movement(1, One_step);
            printf("AUBO i5 turn 60°\r\n");
          }
          else if(buff[2] == 0x30 && buff[3] == 0x32)
          {
            zenan_demo.target_movement(2, One_step);
            printf("AUBO i5 turn 120°\r\n");
          }
          else if(buff[2] == 0x30 && buff[3] == 0x33)
          {
            zenan_demo.target_movement(3, One_step);
            printf("AUBO i5 turn -z \r\n");
          }
          else if(buff[2] == 0x30 && buff[3] == 0x34)
          {
            zenan_demo.target_movement(4, One_step);
            printf("AUBO i5 turn +z \r\n");
          }
          else if(buff[2] == 0x30 && buff[3] == 0x35)
          {
            zenan_demo.target_movement(5, One_step);
            printf("AUBO i5 turn -r \r\n");
          }
          else if(buff[2] == 0x30 && buff[3] == 0x36)
          {
            zenan_demo.target_movement(6, One_step);
            printf("AUBO i5 turn +r \r\n");
          }
      }

      // 04-100hz must to match the send rate.
      usleep(10000);
    }

    return 0;
}
