//socket udp 服务端
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

int unPack()
{

}

size_t packRecv(unsigned char *buf, int sockfd, struct sockaddr_in cli, socklen_t len)
{
	unsigned char tempBuf[60006];
	uint8_t pkgCount;
	uint8_t curtPkg;
	size_t datalen;
	unsigned char *ptr = tempBuf;
	bzero(buf, sizeof(buf));

	recvfrom(sockfd,tempBuf,sizeof(tempBuf),0,(struct sockaddr*)&cli,&len);
	
	pkgCount = *ptr;
	ptr++;
	curtPkg = *ptr;
	ptr++;
	datalen = *((size_t *)ptr);
printf("pkgCount: %d curtPkg: %d datalen: %d\n", pkgCount, curtPkg, datalen);
	if(pkgCount == 1)
	{
		memcpy(buf, tempBuf+6, datalen);
	}else if(pkgCount > 1)
	{
		memcpy(buf, tempBuf+6, 60000);	//the first pkg has already received
printf("1111111\n");
		unsigned char *ptr = buf + 60000;
		for(int i=2;i<=pkgCount;i++)
		{
			bzero(tempBuf, sizeof(tempBuf));
			if(i < pkgCount)
			{
				recvfrom(sockfd,tempBuf,sizeof(tempBuf),0,(struct sockaddr*)&cli,&len);
				//verify id
				if(tempBuf[1] != i)
				{
printf("1.5\n");
					//return -1;
				}
				memcpy(ptr, tempBuf+6, 60000);
printf("2222222\n");
				ptr += 60000;
			}else if(i == pkgCount)
			{
				size_t lastData = datalen-((i-1)*60000);
				recvfrom(sockfd,tempBuf, lastData,0,(struct sockaddr*)&cli,&len);
				if(tempBuf[1] != i)
				{
printf("2.5\n");
					//return -1;
				}
				memcpy(ptr, tempBuf+6, lastData);
printf("3333333\n");
			}else
			{
printf("3.5\n");
				return -1;
			}
		}
	}else
	{
printf("4.5\n");
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
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");

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
		size_t datalen = packRecv(buf, sockfd, cli, len);

printf("recv datalen: %d\n", datalen);
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
