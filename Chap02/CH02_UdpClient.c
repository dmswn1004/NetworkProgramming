#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
 
#define BUF_SIZE 100
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	char message[BUF_SIZE] = "abcdefghijklmnopqrstuvwxyz";
	int strLen, rcvLen, clntAdrSz;
	
	SOCKADDR_IN servAdr, PeerAddr;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	servAdr.sin_port		= htons(9000);
	
	for (int index = 0; index < 5; index++)
	{
		strLen = strlen(message);
		sendto(sock, message, strLen, 0, (SOCKADDR*)&servAdr, sizeof(servAdr));
		printf("%d> sent UDP message:%s\n", index+1, message);

		clntAdrSz = sizeof(PeerAddr);
		rcvLen = recvfrom(sock, message, sizeof(message), 0, (SOCKADDR*)&PeerAddr, &clntAdrSz);
		//message[rcvLen] = 0;
		//printf("Message from server: %s", message);
		Sleep(1000);
	}	
	closesocket(sock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
