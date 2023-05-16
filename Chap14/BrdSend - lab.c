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
	// ��۱�ó��...
	// 1-2�� ���� �ݺ�...
	// 1. ��� ���� �о
	// 2. Ư�� multicast IP (��� ä��) �ּҷ� ����. 
	// multicast �� IP�ּ� = D class 1110~  224 ~  (224.1.1.2)

	// BROADCASTING Ȱ��ȭ �����ϱ� 
	ret = setsockopt(	hSendSock, SOL_SOCKET, SO_BROADCAST, 
				(void*)&so_brd, sizeof(so_brd) );
	if (ret == SOCKET_ERROR) {
		ErrorHandling("setsockopt(SO_BRD) error");
	}

	// ��Ƽĳ��Ʈ �ּ� �����ϱ�..
	memset(&mulAdr, 0, sizeof(mulAdr));
	mulAdr.sin_family		= AF_INET; // IPv4
	mulAdr.sin_addr.s_addr	= inet_addr("255.255.255.255"); // local broadcast
	mulAdr.sin_port			= htons(9000);

	// Loop (���� �о Multi�ּҷ� �����ϱ�)
	// ���� ����
	fp = fopen("data.txt", "r");
	if ( fp == NULL ) {
		ErrorHandling("���� ���� ����");
	}

	while (1)
	{
		// ���� �б�
		fgets( buf, BUF_SIZE, fp );

		// mulAdr �ּҷ� ���� �ϱ�
		sendto( hSendSock, buf, strlen(buf), 0,
				(SOCKADDR*)&mulAdr, sizeof(mulAdr) );

		printf("%s", buf);
		Sleep(2000);

		// EOF ����...
		if ( feof(fp) ) {
			// �� ������ ������ �̵�...
			fseek( fp, 0, SEEK_SET ); // �� ������ �̵�...
			printf("\n ���� �����͸� ó������ �̵�...\n");
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