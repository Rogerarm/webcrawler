
#ifndef URLNODE_H_
#define URLNODE_H_

#include<string>
using namespace std;

struct URLNode
{
	string strURL;       //URL
	int    nPort;        //URL port
	string strAttri;     //URL attribute
	string strName;      //URL name
	string HtmlUrl;
	int    NodeDepth;
};
#endif
