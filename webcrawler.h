//webcrawler.h is written by roger on July 13Sat,2013

#ifndef WEBCRAWLER_H_
#define WEBCRAWLER_H_

#include "urlnode.h"
#include <string>
#include <queue>
#include <cstdbool> 

using namespace std;

class webcrawler
{
	public:
		webcrawler(URLNode nodetemp,string FileNametemp);
		~webcrawler();
		bool downloadHTMLFile();
		bool analysisComment();
		queue<URLNode> HtmlUrlQueue;
	private:
		string FileName;
		string strInfor;
		URLNode node;
		void getURLInfor(string strSource,string &strURL,int &nPort,string &strAttri);
		int setnonblocking(int sockfd);
};
#endif
