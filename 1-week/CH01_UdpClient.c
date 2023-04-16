#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
 
#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET sock;
	char message[BUF_SIZE];
	int strLen, rcvLen, clntAdrSz;
	
	SOCKADDR_IN servAdr, PeerAddr;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	sock=socket(PF_INET, SOCK_DGRAM, 0);   
	if(sock==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servAdr.sin_port=htons(9000);
	
	//connect(sock, (SOCKADDR*)&servAdr, sizeof(servAdr));

	while(1)
	{
		fputs("Insert message(q to quit): ", stdout);
		fgets(message, sizeof(message), stdin);     
		if(!strcmp(message,"q\n") || !strcmp(message,"Q\n")) break;

		sendto(sock, message, sizeof(message), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));

		//send(sock, message, strlen(message), 0);
		//strLen=recv(sock, message, sizeof(message)-1, 0);
		clntAdrSz = sizeof(PeerAddr);
		rcvLen = recvfrom(sock, message, sizeof(message), 0, (SOCKADDR*)&PeerAddr, &clntAdrSz);
		
		strLen = strlen(message);
		message[strLen] = 0;
		printf("Message from server: %s", message);
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
