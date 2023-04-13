#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
void ErrorHandling(char *message);
void ShowSocketBufSize(SOCKET sock);
void GetSocketBufSize(SOCKET sock, int *sendBuf, int *rcvBuf );

int main(int argc, char *argv[])
{
	WSADATA  wsaData;
	SOCKET hSock;
	int sndBuf, rcvBuf, state;
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	
	hSock = socket(PF_INET, SOCK_STREAM, 0);
	ShowSocketBufSize(hSock);

	GetSocketBufSize(hSock, &sndBuf, &rcvBuf);
	
	sndBuf 	= sndBuf*2;
	rcvBuf 	= rcvBuf*2;
	
	//	새로운 버퍼 크기 설정하기
	state = setsockopt(hSock, SOL_SOCKET, SO_SNDBUF, (char*)&sndBuf, sizeof(sndBuf));
	if(state == SOCKET_ERROR){
		ErrorHandling("<ERROR. 1> setSockopt() error");
	}
	state = setsockopt(hSock, SOL_SOCKET, SO_RCVBUF, (char*)&rcvBuf, sizeof(rcvBuf));
	if(state == SOCKET_ERROR){
		ErrorHandling("<ERROR. 2> setSockopt() error");
	}
	
	ShowSocketBufSize( hSock );
	
	closesocket(hSock);
	WSACleanup();
	return 0;
}
 
void GetSocketBufSize(SOCKET sock, int *sendBuf, int *rcvBuf )
{
	int tmpSndBuf, tmpRcvBuf, state, len;

	// 버퍼 크기 확인하기
	// 송신 버퍼 크기 확인
	len = sizeof(tmpSndBuf);
	state = getsockopt(sock, SOL_SOCKET,SO_SNDBUF, (char*)&tmpSndBuf, &len);
	if(state == SOCKET_ERROR){
		ErrorHandling("<ERROR. 1> getsockopt() error");
	}

	// 수신 버퍼 크기 확인
	len = sizeof(tmpRcvBuf);
	state = getsockopt(sock, SOL_SOCKET,SO_RCVBUF, (char*)&tmpRcvBuf, &len);
	if(state == SOCKET_ERROR){
		ErrorHandling("<ERROR. 2> getsockopt() error");
	}
	*sendBuf = tmpSndBuf;
	*rcvBuf = tmpRcvBuf;
}

void ShowSocketBufSize(SOCKET sock)
{
	int sndBuf, rcvBuf, state, len;
	len = sizeof(sndBuf);
	// 버퍼 크기 보여주기
	// 송신 버퍼 크기 확인
	state = getsockopt(sock, SOL_SOCKET,SO_SNDBUF, (char*)&sndBuf, &len);
	if(state == SOCKET_ERROR){
		ErrorHandling("<ERROR. 1> getsockopt() error");
	}
	len = sizeof(rcvBuf);
	state = getsockopt(sock, SOL_SOCKET,SO_RCVBUF, (char*)&rcvBuf, &len);
	if(state == SOCKET_ERROR){
		ErrorHandling("<ERROR. 2> getsockopt() error");
	}
	printf("> Send buffer size = %d  bytes\n", sndBuf);
	printf("> Rcev buffer size = %d  bytes\n", rcvBuf);
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}