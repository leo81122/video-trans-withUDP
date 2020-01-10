//socket udp 客户端
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
//#include <boost/filesystem.hpp>
//#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

int packUp(uint8_t pkgCount, uint8_t curtPkg, size_t datalen, unsigned char *tempBuf)
{
	/*
	uint8_t pkgCount
	uint8_t curtPkg
	size_t  datalen
	 */
	unsigned char* ptr = tempBuf;
	*ptr = pkgCount;
	ptr++;
	*ptr = curtPkg;
	ptr++;
	*((size_t *)ptr) = datalen;
printf("datalen in packup:%d\n", datalen);
}

int packSend(size_t datalen, unsigned char *buf, int sockfd, struct sockaddr_in addr)
{
	int pkgCount = datalen/60000 + 1;
	unsigned char tempBuf[60006];

printf("pkgCount = %d\n", pkgCount);
	if (pkgCount == 1)
	{
		bzero(tempBuf, sizeof(tempBuf));
		memcpy(tempBuf+6, buf, datalen);
		packUp(1, 1, datalen, tempBuf);
		sendto(sockfd,tempBuf,datalen,0,(struct sockaddr*)&addr,sizeof(addr));
	}else if(pkgCount > 1)
	{
		unsigned char *ptr = buf;
		for(int i=1; i<=pkgCount; i++)
		{
			bzero(tempBuf, sizeof(tempBuf));
			if(i < pkgCount) //not the last pkg
			{
				memcpy(tempBuf+6, ptr, 60000);
				packUp(pkgCount, i, datalen, tempBuf);
				sendto(sockfd,tempBuf,sizeof(tempBuf),0,(struct sockaddr*)&addr,sizeof(addr));
				ptr += 60000;
			}else if(i == pkgCount) //the last  pkg
			{
				size_t lastData = datalen-((i-1)*60000);
				memcpy(tempBuf+6, ptr, lastData);	
				packUp(pkgCount, i, datalen, tempBuf);
				sendto(sockfd,tempBuf,lastData,0,(struct sockaddr*)&addr,sizeof(addr));
			}else
			{
				return -1;
			}
		}
	}else
	{
		return -1;
	}
	return 0;
}

int main()
{
	//创建socket对象
	int sockfd=socket(AF_INET,SOCK_DGRAM,0);

	//创建网络通信对象
	struct sockaddr_in addr;
	addr.sin_family =AF_INET;
	addr.sin_port =htons(9999);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//store image, space:1MB
	unsigned char buf[1000000];

	VideoCapture capture("1.mp4");

	while(1)
	{
		//trans 1 frame into jpg, all in buf
		bzero(buf, sizeof(buf));
		Mat tstMat;
		capture.read(tstMat);
resize(tstMat, tstMat, Size(1280, 720), (0, 0), (0, 0), INTER_LINEAR);
		//imshow("picture",tstMat);	//show original img
		vector<unsigned char> inImage;
		imencode(".jpg",tstMat,inImage);
		size_t datalen=inImage.size();
printf("datalen: %d\n", datalen);
		for(int i=0;i<datalen;i++)
		{
			buf[i]=inImage[i];
		}
	                         
		packSend(datalen, buf, sockfd, addr);


		vector<unsigned char> buff;
		for(int i=0;i<datalen;i++)
		{
			buff.push_back(buf[i]);
		}
		Mat show=imdecode(buff,CV_LOAD_IMAGE_COLOR);
		imshow("send",show);
		waitKey(1);
	}
	close(sockfd);

}
