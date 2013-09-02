#include"webcrawler.h"
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<unistd.h>
#include<sys/epoll.h>       /* epoll function */
#include<fcntl.h>           /* nonblocking */

#include<iostream>
#include<cstdio>
#include<cerrno>
#include<cstdlib>
#include<string>
#include<cstring>
#include<fstream>

using namespace std;

webcrawler::webcrawler(URLNode nodetemp,string FileNametemp)
{
	node = nodetemp ;
	FileName=FileNametemp;
}

webcrawler::~webcrawler()
{
}

int webcrawler::setnonblocking(int sockfd)
{
	if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL, 0)|O_NONBLOCK) == -1) 
	{
		return -1;
	}
	return 0;
}
bool webcrawler::downloadHTMLFile()
{
	ofstream HtmlFile;
	const char * filept=FileName.c_str();
	HtmlFile.open(filept,ios::trunc);
	int ds_ClientSocket ;
	struct sockaddr_in ds_ServerAddr ;
	struct epoll_event ev;
	struct epoll_event events[5],eventsrecv[5];
	struct hostent *h = NULL; 
	h=gethostbyname(node.strURL.c_str());

	if (h == NULL) {
		cout<<"get dns failed"<<endl;
		return false;
	}
	//cout<<"Host name :"<< h->h_name<<endl;
	//cout<<"IP Address :"<<inet_ntoa(*((struct in_addr *)h->h_addr))<<endl; 

	ds_ServerAddr.sin_family = AF_INET;
	ds_ServerAddr.sin_port = htons(80);
	ds_ServerAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
	ds_ClientSocket = socket(AF_INET,SOCK_STREAM,0);
	
	if(ds_ClientSocket< 0)
	{
		cout<<"socket init error"<<endl;
		return false;
	}
	int opt = 1;
	setsockopt(ds_ClientSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	if (setnonblocking(ds_ClientSocket) < 0) 
	{
		cout<<"socket setnonblocking error"<<endl;
		return false;
	}
	int ret= connect(ds_ClientSocket,(struct sockaddr *)&ds_ServerAddr,sizeof( ds_ServerAddr ) );
	//cout<<"ret="<<ret<<endl;
	while (ret < 0)
	{
		if (errno == EINPROGRESS) 
		{
			break;
		}
		else
		{
			cout<<"socket connect error"<<endl;
			return false;
		}
	}
	int kdpfd = epoll_create(5);
	ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
	ev.data.fd = ds_ClientSocket;
	if (epoll_ctl(kdpfd, EPOLL_CTL_ADD, ds_ClientSocket, &ev) < 0) 
	{
		cout<<"socket epoll_ctl error"<<endl;
		return false;
	}
	int curfds = 1;
	int nfds = epoll_wait(kdpfd, events, curfds, 1200);
	if (nfds == -1)
	{
		cout<<"epoll_wait error!"<<endl;
		return false;
	}

	if (events[0].data.fd == ds_ClientSocket) 
	{	
		char strRequest[1000];
		sprintf(strRequest,"GET %s HTTP/1.1\r\nHost: %s\r\nAccept: */*\r\n\r\n\r\n",node.strAttri.c_str(),node.strURL.c_str());
		ret = send(ds_ClientSocket,strRequest,sizeof(strRequest),0);
		if(ret < 0)
		{
			close(ds_ClientSocket);
			return false;
		}
		cout<<strRequest<<endl;	
		while(1)
		{
			nfds = epoll_wait(kdpfd, eventsrecv, curfds, 1200);
			if (nfds == -1)
			{
				cout<<"epoll_wait error!"<<endl;
				return false;
			}
			if (eventsrecv[0].data.fd == ds_ClientSocket) 
			{
				int rs=1;
				while(rs)
				{
					char buf[2048];
					bzero(buf,sizeof(buf));
					ret = recv(ds_ClientSocket,buf,2047,0);
					if(ret>0)
					{
						strInfor += buf;
						//cout<<"ret="<<ret<<endl;
						//cout<<"buflen="<<strlen(buf)<<endl;
					}
					else if(ret==0)
					{
						rs=0;
						HtmlFile<<strInfor<<endl;	
						HtmlFile.close();
						close(ds_ClientSocket);
						if(node.NodeDepth<2)
							return true;
						else
							return false;
					}
					else
					{
						if(errno==EAGAIN)
							break;
						else
							rs=0;
					}	
				}
			}	
		}
	} 
}


bool webcrawler::analysisComment()
{
	if(strInfor.size()==0)
		return false;
	string strRule="a href=\"http://";
	cout<<"rule:"<<strRule<<strInfor.size()<<endl;
	unsigned int count = 0;
	char c = strInfor[count++];
	while(count < strInfor.size())
	{
		if( c == '<')
		{
			int i = 0;
			for(; i < 15; i++)
			{
				c = strInfor[count++];
				if(strRule[i] != c)
					break;
			}
			if(i == 15)
			{
				string strURL;
				c = strInfor[count++];
				while(count < strInfor.size())
				{
					if(strURL.size() > 300)
					{
						i = 0;
						break;
					}

					if(c == '"' )
					{
						break;
					}
					strURL += c;
					c = strInfor[count++];
				}
				if( i > 0)
				{
					c = strInfor[count++];
					i = 0;
					while(count < strInfor.size())
					{
						i++;
						if( i > 300)
						{
							i = 0;
							break;
						}
						if(c == '>')
							break;
						c = strInfor[count++];
					}
					if(i > 0)
					{
						c = strInfor[count++];
						string strName;
						i = 0;
						while(count < strInfor.size())
						{
							if(strName.size() > 50)
								break;

							if(c == '<')
								break;

							strName += c;
							c = strInfor[count++];
						}
						//cout<<"add a new url !"<<endl;
						URLNode NewNode;
						getURLInfor(strURL,NewNode.strURL,NewNode.nPort,NewNode.strAttri);
						NewNode.strName = strName;
						NewNode.HtmlUrl = strURL;
						NewNode.NodeDepth = node.NodeDepth+1;						
						HtmlUrlQueue.push(NewNode);
						cout<<"the new url's nodedepth"<<NewNode.NodeDepth<<endl;
					}
				}			
			}			
		}
		c =	strInfor[count++];
	}
	return true;
}


void webcrawler:: getURLInfor(string strSource,string &strURL,int &nPort,string &strAttri)
{
	int i;
	for( i = 0; i < strSource.size(); i++)
	{
		if(strSource[i] == ':')
		{
			//cout<<"check num...."<<endl;
			char num[6];
			char c = strSource[++i];
			int j = 0;
			while(c != '\0' && c != '/')
			{
				num[j++] = c;
				c = strSource[++i];
			}
			num[j] = '\0';
			nPort = atoi(num);
			break;
		}

		if(strSource[i] == '/')
			break;

		strURL += strSource[i];		
	}

	for(; i <strSource.size(); i++)
	{
		strAttri += strSource[i];
	}
}
