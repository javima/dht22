/*
 * Copyright © 2016 Javier Martínez Baena (jbaena@ugr.es)
 *
 * This file is part of DHT_11_22
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
#include <iostream>
#include <string>
#include <cstdlib>
#include "tclap/CmdLine.h"
#include "DHT_11_22.h"

using namespace std;

int main(int argc, char *argv[])
{
    // Start wiringPi lib with GPIO numbering
    if (wiringPiSetupGpio()==-1) {
        cout << "Error initializing RPi" << endl;
        exit(1);
    }

    try {
        TCLAP::CmdLine cmd("Test for DHT sensors", ' ', "1.0");
        TCLAP::ValueArg<string> arg_type("t","type","Type of DHT sensor [11|22]",true,"11","Type of DHT sensor [11|22]");
        TCLAP::ValueArg<int> arg_pin("p","pin","GPIO pin number",true,-1,"GPIO pin number");
        TCLAP::ValueArg<int> arg_delay("d","delay","Delay in ms between readings",false,1000,"Delay in ms between readings");
        cmd.add(arg_type);
        cmd.add(arg_pin);
        cmd.add(arg_delay);
        cmd.parse(argc,argv);
        
        DHTSensor *sen;
        if (arg_type.getValue()=="11")
            sen = new DHT11Sensor(arg_pin.getValue());
        else
            sen = new DHT22Sensor(arg_pin.getValue());
            
        while (true) {
            bool ok = sen->read();
            cout << "Data read (" << (ok?"OK ":"ERR") << ")   "
                 << "Temperature: " << sen->lastTemperature() << " ºC       "
                 << "Humidity: " << sen->lastHumidity() << " %RH" << endl;
            delay(arg_delay.getValue());
        }
        
        delete sen;
    } catch (TCLAP::ArgException &e) {
        cerr << "Error: " << e.error() << " for arg " << e.argId() << endl;
    }
}
