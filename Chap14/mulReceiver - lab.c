#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>     // for struct ip_mreq

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hRecvSock;
	SOCKADDR_IN		adr;
	struct			ip_mreq	joinAdr;
	char			buf[BUF_SIZE];
	int				strLen, ret;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 
  
	hRecvSock = socket(PF_INET, SOCK_DGRAM, 0);
 	
	// 멀티개스트 주소로 전송된 데이터(멀티캐스트 데이터) 수집
	
	// 1. UDP bind
	memset(&adr, 0, sizeof(adr));
	adr.sin_family = AF_INET;
	adr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	adr.sin_port = htons(9000);

	ret = bind(hRecvSock, (SOCKADDR*)&adr, sizeof(adr));
	if (ret == SOCKET_ERROR) {
		ErrorHandling("bind() error");
	}

	// 2. 멀티캐스트 수신을 위한 membership 가입 ... setsockopt()
	joinAdr.imr_multiaddr.S_un.S_addr = inet_addr("224.1.1.2");
	joinAdr.imr_interface.S_un.S_addr = htonl(INADDR_ANY);
	ret = setsockopt(hRecvSock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&joinAdr, sizeof(joinAdr));
	if (ret == SOCKET_ERROR) {
		ErrorHandling("setsockopt() error");
	}
	 
	// 3.  수신 반복 ...
	while (1)
	{
		strLen = recvfrom(hRecvSock, buf, BUF_SIZE - 1, 0, NULL, 0);
		if (strLen < 0) {
			printf("ERROR : 수신 오류\n");
			break;
		}
		buf[strLen] = 0;
		printf("%s",buf);
	}


	closesocket(hRecvSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}