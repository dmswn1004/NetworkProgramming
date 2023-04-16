#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET servSock;
	char message[BUF_SIZE];
	int strLen, rcvLen;
	int clntAdrSz;
	
	SOCKADDR_IN servAdr, clntAdr;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	servSock=socket(PF_INET, SOCK_DGRAM, 0);
	if(servSock==INVALID_SOCKET)
		ErrorHandling("UDP socket creation error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(9000);
	
	if(bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	while(1) 
	{
		clntAdrSz = sizeof( clntAdr );
		rcvLen = recvfrom( servSock, message, sizeof(message), 0,
						   (SOCKADDR*)&clntAdr, &clntAdrSz );
		strLen = strlen(message);
		message[strLen] = '\0';
		printf("message from client(IP:%s,PORT:%d) : %s \n", 
			   inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port), message );

		sendto( servSock, message, sizeof(message), 0, 
				(SOCKADDR*)&clntAdr, sizeof(clntAdr) );
	}	
	closesocket(servSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}