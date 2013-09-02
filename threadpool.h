//threadpool.h is written by roger on July 04th,2013

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>
//#include <semaphore.h>
#include <queue>
#include <unordered_set>
#include <string>

#include "urlnode.h"
using namespace std;

class ThreadPool
{
	public:
    		ThreadPool();
    		ThreadPool(int maxThreadsTemp);
    		virtual ~ThreadPool();
	
		void destroyPool(int maxPollSecs);

		void initializeThreads();
	
    		static void *threadExecute(void *param);
    
    		static pthread_mutex_t mutexUrlQueue;
    		static pthread_mutex_t mutexUrlHash;
    		static pthread_mutex_t mutexFileIndex;
	private:
    		int maxThreads;
    		static int FileIndex;
    		static string FileName;
		static unordered_set<string> *UrlHash;
    		static queue<URLNode> *UrlQueue;
		static queue<URLNode> UrlQueueTemp;
};

#endif
