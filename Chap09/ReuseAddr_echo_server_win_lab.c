#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	char message[BUF_SIZE];
	int strLen, i=0;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSize;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	hServSock=socket(PF_INET, SOCK_STREAM, 0);   
	if(hServSock==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family			= AF_INET;
	servAdr.sin_addr.s_addr		= htonl( INADDR_ANY );
	servAdr.sin_port			= htons( 9000 );

	int optlen, option = TRUE, state;
	optlen = sizeof(option);

	// SO_REUSEADDR 옵션 설정하기
	state = setsockopt(hServSock, SOL_SOCKET, SO_REUSEADDR, (char*)&option, optlen);
	if (state == SOCKET_ERROR) {
		ErrorHandling("<ERROR> so-reuseaddr() error()");
	}

	if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 2)==SOCKET_ERROR)
		ErrorHandling("listen() error");
	
	clntAdrSize	= sizeof( clntAdr );
	hClntSock  = accept( hServSock, (SOCKADDR*)&clntAdr, &clntAdrSize );
	if( hClntSock==-1 )
		ErrorHandling("accept() error");
	else
		printf("%d'th Client is connected.\n", i+1);
	 
	while( ( strLen = recv( hClntSock, message, BUF_SIZE, 0 ) ) != 0 )
	{
		//strLen = recv( hClntSock, message, BUF_SIZE, 0 );
		send(hClntSock, message, strLen, 0);
		message[strLen] = '\0';
		printf("send msg(%d) : %s  to (IP:%s, Port:%d)\n", 
			strLen, message, inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port) );
	}
	closesocket(hClntSock);
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}