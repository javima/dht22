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

#include <wiringPi.h>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include "DHT_11_22.h"

using namespace std;

DHTSensor::DHTSensor(int p, int buftam)
{
    pin=p;
    maxchanges=40;     // Each transmission hace 40 bits
    data[0]=data[1]=data[2]=data[3]=data[4]=0;   // Reset data
    
    // Initialize last readed values. They should be updated with a first call to read() method
    lasttemp = -100;
    lasthum = -100;

    // Initial calibration
    calibrate();

    // Create buffers
    bufferinit = false;
    buffertam = (buftam<0?0:buftam);
    buffer_temp = new BufferC<double>(buffertam);
    buffer_hum = new BufferC<double>(buffertam);
    maxTempDevMedian = 0.2;   // 20%
    maxHumDevMedian = 0.2;    // 20%
}

DHTSensor::~DHTSensor()
{
    delete buffer_temp;    
    delete buffer_hum;    
}

// Adjust some internal parameters:
//   maxiter
//   threshold
bool DHTSensor::calibrate()
{
    int calibnum = 5;   // Maximum number of calibrating iterations
    
    maxiter=1000;       // Start value (needed for waitWhileValue())
    threshold = -1;     // Not yet calibrated ...
    
    do {
        // Reset data
        data[0]=data[1]=data[2]=data[3]=data[4]=0;
        
        // Send command to DHT to start reading
        init();
        
        // Skip initial DHT response
        waitWhileValue(LOW);
        waitWhileValue(HIGH);
        
        // Read data and calculate:
        //   - the number of iterations needed to read LOW level
        //   - maximum number of iterations needed to read a HIGH level
        int sumlow=0, maxhigh=0;
        for (int i=0; i<maxchanges; i++) {
            sumlow += waitWhileValue(LOW);
            int nhigh = waitWhileValue(HIGH);
            if (nhigh>maxhigh)
                maxhigh = nhigh;
        }
        
        // If data is ok then finish calibration process
        if (checksumValid()) {
            // The threshold to discriminate between logical 0 and 1 will be
            // the medium value of the number of iterations needed to read LOW states
            // ¿Why? According DHT specification:
            //   0 = LOW-50us followed by HIGH-26-28us
            //   1 = LOW-50us followed by HIGH-70us
            // Then, the 50us can be a good threshold to separate 26-28 from 70
            threshold = sumlow/maxchanges;
            
            // To read an state we have needed, as much, maxiter iterations
            // We add a 25% 
            maxiter = maxhigh + maxhigh*0.25;   // Maximum number of iterations needed + 25%
            
            // Finish calibrate stage
            calibnum = 0;
        }
        
        // Wait
        //delayMicroseconds(50);
        delay(1);
        
        calibnum--;
        
    } while (calibnum>0);  // Calibrate again if data isn't ok
  
    if (threshold==-1) {
        threshold = 16;      // Seen on some code from the internet
        return false;
    } else
        return true;
}

void DHTSensor::init() const
{
    // Default status for the DATA pin is HIGH, so MCU must pull down
    // the pin to start communications
    pinMode(pin, OUTPUT);      // Set pin mode to OUTPUT
    digitalWrite(pin, LOW);    // Send START signal
    delay(18);                 // Wait 18 mili-s
    digitalWrite(pin, HIGH);   // Send PULL UP
    delayMicroseconds(20);     // Wait 40 micro-s
    pinMode(pin, INPUT);       // Set pin mode to INPUT  
}

int DHTSensor::waitWhileValue(int stat) const
{
    int cont=0;
    while (digitalRead(pin)==stat && cont<maxiter) {
        delayMicroseconds(1);
        cont++;
    }
    return cont;
}

bool DHTSensor::checksumValid() const
{
    return ((data[0]+data[1]+data[2]+data[3])&0xFF) == data[4];
}

unsigned int DHTSensor::readBit() const
{
    int t1 = waitWhileValue(LOW);    // Count LOW iterations
    int t2 = waitWhileValue(HIGH);   // Count HIGH iterations
    if (t1!=maxiter && t2!=maxiter)
        if (t2>threshold)            // Check if logical value is 0 or 1
            return 1;
        else
            return 0;
    else
        return -1;                   // Error
}

bool DHTSensor::read()
{
    // Fill values in buffers the first time this method is called
    // This can't be done in constructor because it calls virtual methods
    if (!bufferinit && buffertam>0) {
        bufferinit=true;
        for (int i=0; i<buffer_temp->size(); i++)
            if (readData(100,1)) {
                buffer_temp->addElement(data2Temp());
                buffer_hum->addElement(data2Hum());
            }
    }

    // Perform data lecture
    // Maximum of 100 tries to read and wait 1ms between consecutive readings
    bool okread=readData(100, 1);

    // If CRC if correct check if the value is similar to the last values readed
    bool oktemp=true, okhum=true;
    if (okread && buffertam>0) {
        
        // Check if temperature is near the median of the last readed values
        double tread = data2Temp();
        double tmed=buffer_temp->medianValue();
        if (fabs(tread-tmed)>tmed*maxTempDevMedian)
            oktemp = false;
        else
            lasttemp = tread;   // Update temperature value
       
        // Check if humidity is near the median of the last readed values
        double hread = data2Hum();
        double hmed=buffer_hum->medianValue();
        if (fabs(hread-hmed)>hmed*maxHumDevMedian)
            okhum = false;
        else
            lasthum = hread;    // Update humidity value

        // Store values even if they are not correct accorging to the median criterion
        // It may occurs that there is a big change in ambiental conditions and values are really
        // far away from previous ones. That's the reason to store it ... when buffer stores several
        // of them, the median will change
        buffer_temp->addElement(tread);
        buffer_hum->addElement(hread);
    }

    return okread && oktemp && okhum;    // true if everything is ok, false otherwise
}

bool DHTSensor::allZeros() const
{
    return (data[0]==0 && data[1]==0 && data[2]==0 && data[3]==0 && data[4]==0);
}

bool DHTSensor::readData(int success, int readdelay)
{
    // success is 0 if you want to read until CRC success
    // success is >0 if a maimum number of tries is desired in case of CRC fail
    assert(success>=0);

    bool okread=false;
    do {
        // Prepare sensor to start data transmision
        init();

        // Skip initial DHT response
        waitWhileValue(LOW);
        waitWhileValue(HIGH);

        // Reset stored data
        data[0]=data[1]=data[2]=data[3]=data[4]=0;

        // Read data
        for (int i=0; i<maxchanges; i++)           // Read MAXCHANGES bits of data
            if (readBit()==1)                      // If logical value is 1 then store on data array
                data[i/8] |= (1 << (7-(i%8)));
            /*else
                data[i/8] &= ~(1 << (7-(i%8)));*/  // Not neccessary if initialized with zero
        
        // Check if data is ok
        if (checksumValid() && !allZeros())
            // Sometimes sensor return all bits with 0 value (0ºC and 0 %HR) and it could be a correct value.
            // But usually this is an error so we fail if this occurs
            okread=true;       // If data is ok then we can finish
        else if (success>1)
            delay(readdelay);  // If we must do more iterations wait some time

        success--;

        //if (!okread) cerr << "   DHT: data error (" << success << ")  :  " << lastTemperature() << endl;
    } while (success!=0 && !okread);

    return okread;
}

// ********************************************

double DHT11Sensor::makeFractional(double n)
{
    while (n>=1)
        n /= 10;
    return n;
}
