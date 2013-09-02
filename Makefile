#Makefile is written by roger on July 04th,2013

webcrawler:main.o threadpool.o webcrawler.o
	g++ main.o threadpool.o webcrawler.o -o webcrawler -pthread

main.o:main.cpp
	g++ -c main.cpp -o main.o -std=c++11

threadpool.o:threadpool.cpp threadpool.h
	g++ -c threadpool.cpp -o threadpool.o -std=c++11

webcrawler.o:webcrawler.cpp webcrawler.h
	g++ -c webcrawler.cpp -o webcrawler.o -std=c++11

clean:
	rm -rf *.o webcrawler
