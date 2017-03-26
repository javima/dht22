# dht22
Code to control DHT11/22 temperature sensor
/*
 * Copyright © 2016 Javier Martínez Baena (jbaena@ugr.es)
 *
 * This file is part of DHT_11_22
 *
 * Based on Daniel Porrey code https://github.com/porrey/dht:
 *  - DHT11 Temperature Sensor project on hackster.io (based on the code posted
 *    by Rahul Kar found at http://www.rpiblog.com/2012/11/interfacing-temperature-and-humidity.html)
 *  - Datasheets of DHT11/22/AM2302
 *
 * DHT_11_22 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DHT_11_22 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with DHT_11_22.  If not, see <http://www.gnu.org/licenses/>.
 */

PREREQUISITES:
  The demo uses the TCLAP library to parse command line arguments. http://tclap.sourceforge.net/
  The code that control sensors needs only the wiringPi library

COMPILE:
  Edit makefile and change directory for TCLAP library
  make

USAGE of the demo:
  sudo ./DHT_11_22_test
  
Some explanations of the code:

Datasheets from Aosong(Guangzhou) Electronics Co.,Ltd. has been used. They have a good explanation about this sensors:
  http://akizukidenshi.com/download/ds/aosong/DHT11.pdf
  http://akizukidenshi.com/download/ds/aosong/AM2302.pdf

How the DHT 11/22 sensors works? (from datasheets):
1.- MCU (Arduino, Raspberry, ...) send a LOW signal and wait for 18 microseconds. Then sends a HIGH signal and wait for 40 milisecons
2.- Now DHT is ready to transmit data:
    2.1.- DHT send LOW for 80 microseconds
          DHT send HIGH for 80 microseconds
    2.2.- DHT send a sequence of 40 bits with data (serial):
          Each bit is composed of a LOW state followed by a HIGH state
            LOW state stands for 50 microseconds
            HIGH state stands for:
              26-28 microseconds if a logical 0 is transmitted
              70 microseconds if a logical 1 is transmitted

This code has some advantages over others:
1.- Is more structurated (from a programming point of view)
2.- Adds functionality to calibrate sensors
3.- Improve error correction

The idea of the algorithm that read data from sensors is: sample inputs from GPIO pin and count how long it is in HIGH state (counting number of samples). The sampling frequency may be different depending on the MCU processor speed or work load. The calibration process counts how many samples are needed to read a LOW sequence (LOW sequences are 50 microseconds). This count is used as threshold to differenciate between logial 0's or 1's (the number of samples for 0 is lower than threshold and the number of samples for 1 is higher than threshold).

The algorithm used to determine the lenght of a state (LOW or HIGH) is: read pin while its value isn't changed. Sometimes errors occurs and it is not assured that a state change may occur. We need to stop the process if we reach a sufficiently high number of samples. During the calibration process, this limit is established to the number of samples of a HIGH state plus a 25%.

Also, there may be some bug in this code: it has been proved only with 2 cheap sensors (DHT11 and DHT22) and I don't know if it works with others. For example, my DHT11 always have a 0 as the decimal part of temperature and humidity ¿is that right for every DHT11? Also, it has not been proved for negative temperatures.

Improvements related to error correction are:
1.- Sensor reading is repeated until a correct CRC is found. There are also a maximum number of readings. So, when you call the read data method it is almost secured that the readed bits have a correct CRC.
2.- Sometimes the sensor returns a list of 0's including data and CRC. In this cases CRC is ok but data is not (with a high probability). If this occurs, this software detects it as a lecture error.
3.- Even if the CRC is ok and the data is not 0's, it is not assured that readings are correct. The CRC method used is very simple and can fail easily giving wrong values as correct. This software uses a buffer (one for temperature and another for humidity) that stores last readed values. When CRC is ok it checks if the value is similar to the median value of values stored in buffer. If it is very different then it is treated as an error. If you do repeated lectures in a short period of time, all of them should be similar (there shouldn't be great ambiental changes in a short period of time). But what if it is a real great change or if you perform lectures very far away in time ones from another? In this cases, readed values are stored in buffer even if they are far away from the median value; approximately in BuffSize/2 lectures the median will change and the new lectures will be treated as correct. Buffer size should not be very big, a value of 5-11 should be enough depending of the time between consecutive readings. Buffer size can be set to cero in the constructor if you don't want to use it.
