//main.cpp is written by roger on July 04th,2013

#include "threadpool.h"
#include "unistd.h"
#include <iostream>


using namespace std;



int main(int argc, char **argv)
{
	ThreadPool* myPool = new ThreadPool(5);
	myPool->initializeThreads();
    	time_t t1=time(NULL);
	while(1);
	sleep(20);
    	myPool->destroyPool(10);
    	time_t t2=time(NULL);
    	cout << t2-t1 << " seconds elapsed\n" << endl;
	delete myPool;
    	return 0;
}
