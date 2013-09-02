//threadpo.cpp writen by Roger at July.4.2013

#include "threadpool.h"
#include "webcrawler.h"
#include "unistd.h"


#include <cstdlib>
#include <iostream>

using namespace std;

pthread_mutex_t ThreadPool::mutexUrlQueue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ThreadPool::mutexUrlHash = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ThreadPool::mutexFileIndex = PTHREAD_MUTEX_INITIALIZER;
unordered_set<string> *ThreadPool::UrlHash=new unordered_set <string>;
queue<URLNode> *ThreadPool::UrlQueue=new queue<URLNode>;
queue<URLNode> ThreadPool::UrlQueueTemp={};
int ThreadPool::FileIndex=0;
string ThreadPool::FileName={};

ThreadPool::ThreadPool()
{
	ThreadPool(2);
}

ThreadPool::ThreadPool(int maxThreadsTemp)
{
	if (maxThreadsTemp < 1)  maxThreads=1;
   	this->maxThreads = maxThreadsTemp;
}

void ThreadPool::initializeThreads()
{	
   	for(int i = 0; i<maxThreads; ++i)
	{
		pthread_t threadid;
		int temp = pthread_create(&threadid, NULL,&ThreadPool::threadExecute, (void *) this );     
		//if(temp < 0)
		//	cout<<"pthread failed"<<endl;
		//else
		//	cout<<"pthread succed"<<endl;
	}
	URLNode firstnode;
	firstnode.strURL="www.qq.com";
	firstnode.nPort =80;
	firstnode.strAttri ='/';
	firstnode.HtmlUrl="www.qq.com:80/";
	firstnode.NodeDepth=1;
	pthread_mutex_lock(&mutexUrlHash);
		UrlHash->insert (firstnode.HtmlUrl);
	pthread_mutex_unlock(&mutexUrlHash);
	pthread_mutex_lock(&mutexUrlQueue);
		UrlQueue->push(firstnode);
	pthread_mutex_unlock(&mutexUrlQueue);
	system("rm -rf htmlfile");
	system("mkdir htmlfile");
	//cout<<"pthread exit"<<endl;
}

ThreadPool::~ThreadPool()
{
}



void ThreadPool::destroyPool(int maxPollSecs = 5)
{
	while( UrlQueue->size()>0 )
	{
	        cout << "Work is still incomplete=" << UrlQueue->size() << endl;
		sleep(maxPollSecs);
	}
	cout << "All Done!! Wow! That was a lot of work!" << endl;
        pthread_mutex_destroy(&mutexUrlQueue);
        pthread_mutex_destroy(&mutexUrlHash);
	delete UrlHash;
	delete UrlQueue;
	cout << "END   Work is still incomplete=" << UrlQueue->size() << endl;

}

void *ThreadPool::threadExecute(void *param)
{	
	//cout<<"threadExecute enter"<<endl;
	while(1)
	{		
		//cout<<"threadExecute working"<<endl;
		if(UrlQueue->size() <= 0)
		{
			sleep(5);
			continue;
		}
		pthread_mutex_lock(&mutexUrlQueue);
			URLNode node =UrlQueue->front();
			UrlQueue->pop();	
		pthread_mutex_unlock(&mutexUrlQueue);

		pthread_mutex_lock(&mutexFileIndex);
			FileIndex++;
			FileName="./htmlfile/";
			FileName+=to_string(FileIndex)+".html";
			webcrawler* webcrawlerwork= new webcrawler(node,FileName);
		pthread_mutex_unlock(&mutexFileIndex);

		if(!webcrawlerwork->downloadHTMLFile())
		{
			delete webcrawlerwork;
			continue;
		}

		if(!webcrawlerwork->analysisComment())
		{
			delete webcrawlerwork;
			continue;
		}
		UrlQueueTemp=webcrawlerwork->HtmlUrlQueue;
		while(UrlQueueTemp.size() > 0)
		{
			URLNode NodeTemp =UrlQueueTemp.front();
			UrlQueueTemp.pop();
			unordered_set<std::string>::const_iterator got = UrlHash->find (NodeTemp.HtmlUrl);
			if ( got == UrlHash->end() )
		    	{	
				pthread_mutex_lock(&mutexUrlHash);
					UrlHash->insert (NodeTemp.HtmlUrl);
				pthread_mutex_unlock(&mutexUrlHash);
				pthread_mutex_lock(&mutexUrlQueue);
					UrlQueue->push(NodeTemp);
				pthread_mutex_unlock(&mutexUrlQueue);
				//cout<<"insert work succ"<<endl;
			}
		}
		delete webcrawlerwork;
	}
	return NULL;
}
