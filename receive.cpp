//UDP receive
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include <iostream>
#include <sstream>
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

/*
	receive UDP packages and put them together
	return:length of date
*/
size_t packRecv(unsigned char *buf, int sockfd, struct sockaddr_in cli, socklen_t len)
{
	unsigned char tempBuf[60006];
	uint8_t pkgCount;
	uint8_t curtPkg;
	size_t datalen;
	unsigned char *ptr = tempBuf;
	bzero(buf, sizeof(buf));

	recvfrom(sockfd,tempBuf,sizeof(tempBuf),0,(struct sockaddr*)&cli,&len);//recv first pkg
	
	//get pkg info
	pkgCount = *ptr;
	ptr++;
	curtPkg = *ptr;
	ptr++;
	datalen = *((size_t *)ptr);

	//only 1 pkg?
	if(pkgCount == 1)
	{
		memcpy(buf, tempBuf+6, datalen);
	}else if(pkgCount > 1)
	{
		memcpy(buf, tempBuf+6, 60000);	//the first pkg has already received
		unsigned char *ptr = buf + 60000;
		
		//recv last pkgs
		for(int i=2;i<=pkgCount;i++)
		{
			bzero(tempBuf, sizeof(tempBuf));
			if(i < pkgCount)//not the last one
			{
				recvfrom(sockfd,tempBuf,sizeof(tempBuf),0,(struct sockaddr*)&cli,&len);
				//verify id
				if(tempBuf[1] != i)
				{
					//return -1;
				}
				memcpy(ptr, tempBuf+6, 60000);
				ptr += 60000;
			}else if(i == pkgCount) //the last pkg
			{
				size_t lastData = datalen-((i-1)*60000);
				recvfrom(sockfd,tempBuf, lastData,0,(struct sockaddr*)&cli,&len);
				if(tempBuf[1] != i)
				{
					//return -1;
				}
				memcpy(ptr, tempBuf+6, lastData);
			}else
			{
				return -1;
			}
		}
	}else
	{
		return -1;
	}
	return datalen;
}

int main()
{
	//创建socket对象
	int sockfd=socket(AF_INET,SOCK_DGRAM,0);

	//创建网络通信对象
	struct sockaddr_in addr;
	addr.sin_family =AF_INET;
	addr.sin_port =htons(9999);
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");//test ip

	//绑定socket对象与通信链接
	int ret =bind(sockfd,(struct sockaddr*)&addr,sizeof(addr));
	if(0>ret)
	{
		printf("bind\n");
		return -1;
	}
	struct sockaddr_in cli;
	socklen_t len=sizeof(cli);

	//store image
	unsigned char buf[1000000];

	while(1)
	{
		//recv data
		size_t datalen = packRecv(buf, sockfd, cli, len);

		//decode using opencv imdecode
		vector<unsigned char> buff;
		for(int i=0;i<datalen;i++)
		{
			buff.push_back(buf[i]);
		}       
		Mat show=imdecode(buff,CV_LOAD_IMAGE_COLOR);
		imshow("recv",show);
		waitKey(1);
	}
	close(sockfd);
}
