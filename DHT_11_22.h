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

#ifndef DHT_11_22__H
#define DHT_11_22__H

#include "bufferC.h"

// Pure virtual class to manage Temperature and Humidity sensors DHT11, DHT22/AM2302
class DHTSensor {
    protected:
        int pin;                // pin connected to sensor
        int maxchanges;         // 5 bytes, each bit is LOW+HIGH
        int maxiter;            // aprox. maximum iterations to read a state (LOW or HIGH) 
        int threshold;          // threshold to decide if a data is 0 or 1
        unsigned char data[5];  // buffer to read data from sensor
        double lasttemp;        // last correct temperature value 
        double lasthum;         // last correct humidity value
        
        void init() const;                    // Send command to DHT sensor to initiate data transmission
        int waitWhileValue(int stat) const;   // Read data from sensor while state equals to stat, return number of readings
        bool checksumValid() const;           // Return true if CRC of stored/readed data is ok. False otherwise
        unsigned int readBit() const;         // Read one logical bit from sensor (uses waitWhileValue to read LOW followed by HIGH)
        bool allZeros() const;                // Return true is data[] stores only zeros and false otherwise

        // Returns the value stored in data as a double value of temperature
        virtual double data2Temp() const =0;

        // Returns the value stored in data as a double value of humidity
        virtual double data2Hum() const =0;
        
        // Some wrong lectures have correct CRC. To improve this we use buffering
        BufferC<double> *buffer_temp;
        BufferC<double> *buffer_hum;
        int buffertam;              // Size of buffers
        bool bufferinit;            // Are buffers initialized?
        double maxTempDevMedian;    // Maximum deviation from median value (temperature)
        double maxHumDevMedian;     // Maximum deviation from median value (humidity)

        // Read data from sensor
        // Returns true on successfull reading and false in other case
        // success: maximum number of tries in case some of them fails
        //          0: try until read is ok. Limitation: aprox 2*MAXINTEGER
        //          positive number: maximum number of tries
        //          negative number: not allowed
        // readdelay: number of miliseconds to delay in case there are several readings
        bool readData(int success=1, int readdelay=1);

    public:
        // Constructor: initialization os object
        // p: Pin connected to the sensor
        // buftam: size of internal buffer
        DHTSensor(int p, int buftam);
        ~DHTSensor();
                        
        // Read data from sensor. Returns true if success and false otherwise
        bool read();
        
        // Returns the last value of temperature readed correctly
        double lastTemperature() const  { return lasttemp; };

        // Returns the last value of humidity readed correctly
        double lastHumidity() const     { return lasthum; };
        
        // Perform calibration. It is executed during object construction so, usually you don't need to call it
        bool calibrate();
};

// Class to manage DHT11 sensor
class DHT11Sensor : public DHTSensor {
    private:
        static double makeFractional(double n);   // Returns the double value 0.n
        double data2Temp() const    { return data[2] + makeFractional(data[3]); };
        double data2Hum() const     { return data[0] + makeFractional(data[1]); };
        
    public:
        DHT11Sensor(int p, int buftam=11) : DHTSensor(p,buftam) { };
};

// Class to manage DHT22/AM2302 sensor
class DHT22Sensor : public DHTSensor {
    private:
        double data2Temp() const    { return ((data[2]&0x80)?-1:1) * (((int)((data[2]&0x7F))<<8) | (int)(data[3]))/10.0; };
        double data2Hum() const     { return (((int)(data[0])<<8) | (int)(data[1]))/10.0; };    
    
    public:
        DHT22Sensor(int p, int buftam=7) : DHTSensor(p,buftam) { };
};  

#endif
