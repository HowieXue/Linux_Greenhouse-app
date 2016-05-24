//=============================================================
// �ļ����ƣ�tcpcli.c
// ����������Transfer a TCP package and receive a response

//=============================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>						// bzero
#include <unistd.h>						// getopt
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>					// inet_ntop
#include <sys/select.h>

int send_and_recv_via_udp(const char *ip, unsigned short port, const char *sendString, char *recvString)
{
	int sockfd;									// �׽���
	struct sockaddr_in servAddr;				// ��������ַ�ṹ��
	const char *destIP = ip;

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
										// ����UDP�׽���
	if(sockfd < 0)
	{
		perror("socket");
		return -1;
	}
	bzero(&servAddr, sizeof(servAddr));	// ��ʼ����������ַ
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = inet_addr(destIP);
	sendto(sockfd, sendString, strlen(sendString), 0, (struct sockaddr *)&servAddr, sizeof(servAddr));
	int ret = -1;
	do
	{
		struct timeval to;
		to.tv_sec = 0;
		to.tv_usec = 1000;
		fd_set rset;
		FD_ZERO(&rset);
		FD_SET(sockfd, &rset);
		if(select(sockfd + 1, &rset, NULL, NULL, &to) <= 0)
			break;
	    int len = read(sockfd, recvString, 1024);
	    if(len > 0)
	        ret = 0;
	} while(0);
	return ret;
}
