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

#ifndef UTILS__H
#define UTILS__H

//#include <iostream>

template <class T>
class BufferC {
    private:
        T *buffer;
        int numelem;
        int pactual;
        
        static int compare(const void *a, const void *b) {
                return ( *(T*)a - *(T*)b );
            }

    public:
        BufferC(int t){
                numelem = t;
                buffer = new T [numelem];
                pactual = 0;
            };

        ~BufferC() {
                delete [] buffer;
            };

        void addElement(T dat) {
                buffer[pactual] = dat;
                pactual = (pactual+1)%numelem;
            };
            
        T getLast() const {
                return buffer[pactual==0 ? numelem-1 : (pactual-1)];
            };
            
        T medianValue() const {
                T *aux = new T [numelem];
                for (int i=0; i<numelem; i++)
                    aux[i] = buffer[i];
                qsort(aux,numelem,sizeof(T),compare);
                return aux[numelem/2];
            }
            
        int size() const {
                return numelem;
            };
        
        /*void print() const {
                std::cout << "   Buffer (" << numelem << "):  ";
                for (int i=0; i<numelem; i++)
                    std::cout << buffer[i] << " ";
                std::cout << std::endl;
            }*/
};

#endif
