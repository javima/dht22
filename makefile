TCLAP=../../contrib/tclap-1.2.1/include

DEBUGFLAGS = -O2 -DNDEBUG
CC=g++
CFLAGS=-c -Wall $(DEBUGFLAGS) -I$(TCLAP)
LDFLAGS=-lDHT -lwiringPi -L.

all : libDHT.a DHT_11_22_test

libDHT.a : DHT_11_22.o
	ar rcs libDHT.a DHT_11_22.o

DHT_11_22_test : DHT_11_22_test.o libDHT.a
	$(CC) DHT_11_22_test.o $(LDFLAGS) -o DHT_11_22_test
DHT_11_22_test.o : DHT_11_22_test.cpp DHT_11_22.h
	$(CC) $(CFLAGS) DHT_11_22_test.cpp -o DHT_11_22_test.o
DHT_11_22.o : DHT_11_22.cpp DHT_11_22.h bufferC.h
	$(CC) $(CFLAGS) DHT_11_22.cpp -o DHT_11_22.o

clean:
	\rm -f *.o

mrproper: clean
	\rm -f DHT_11_22_test libDHT.a
