#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>     // for IP_MULTICAST_TTL option
#define TTL			15
#define BUF_SIZE	30
void ErrorHandling(char *message);
int main(void)
{
	WSADATA			wsaData;
	SOCKET			hSendSock;
	SOCKADDR_IN		mulAdr;
	int				timeLive = TTL;
	FILE			*fp;
	char			buf[BUF_SIZE];
	int				flag, so_brd = 1, ret;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 
  	
	hSendSock = socket(PF_INET, SOCK_DGRAM, 0);

	//...
	// 방송국처럼...
	// 1-2를 무한 반복...
	// 1. 기사 파일 읽어서
	// 2. 특정 multicast IP (방송 채널) 주소로 전송. 
	// multicast 용 IP주소 = D class 1110~  224 ~  (224.1.1.2)

	// BROADCASTING 활성화 설정하기 
	ret = setsockopt(	hSendSock, SOL_SOCKET, SO_BROADCAST, 
				(void*)&so_brd, sizeof(so_brd) );
	if (ret == SOCKET_ERROR) {
		ErrorHandling("setsockopt(SO_BRD) error");
	}

	// 멀티캐스트 주소 설정하기..
	memset(&mulAdr, 0, sizeof(mulAdr));
	mulAdr.sin_family		= AF_INET; // IPv4
	mulAdr.sin_addr.s_addr	= inet_addr("255.255.255.255"); // local broadcast
	mulAdr.sin_port			= htons(9000);

	// Loop (파일 읽어서 Multi주소로 전송하기)
	// 파일 열기
	fp = fopen("data.txt", "r");
	if ( fp == NULL ) {
		ErrorHandling("파일 열기 실패");
	}

	while (1)
	{
		// 파일 읽기
		fgets( buf, BUF_SIZE, fp );

		// mulAdr 주소로 전송 하기
		sendto( hSendSock, buf, strlen(buf), 0,
				(SOCKADDR*)&mulAdr, sizeof(mulAdr) );

		printf("%s", buf);
		Sleep(2000);

		// EOF 도달...
		if ( feof(fp) ) {
			// 맨 앞으로 포인터 이동...
			fseek( fp, 0, SEEK_SET ); // 맨 앞으로 이동...
			printf("\n 파일 포인터를 처음으로 이동...\n");
		}
	}

	fclose(fp);
	closesocket( hSendSock );
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}