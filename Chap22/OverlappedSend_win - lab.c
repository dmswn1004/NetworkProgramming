#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
void ErrorHandling(char *msg);

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hSocket;
	SOCKADDR_IN		sendAdr;
	WSABUF			dataBuf;
	char			msg[] = "Network is Computer!";
	int				sendBytes = 0, sendBytes2 = 0;
	WSAEVENT		evObj;
	WSAOVERLAPPED	overlapped;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	memset(&sendAdr, 0, sizeof(sendAdr));
	sendAdr.sin_family			= AF_INET;
	sendAdr.sin_addr.s_addr		= inet_addr("127.0.0.1");
	sendAdr.sin_port			= htons(9000);

	if(connect(hSocket, (SOCKADDR*)&sendAdr, sizeof(sendAdr))==SOCKET_ERROR)
		ErrorHandling("connect() error!");
	
	int ret;

	// buf[] 배열 설정...
	dataBuf.buf = msg;
	dataBuf.len = strlen(msg) + 1;

	// overlapped 구조체 설정...
	evObj = WSACreateEvent(); // manual reset...
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;

	// overlapped IO - send
	ret = WSASend(hSocket, &dataBuf, 1, &sendBytes, 0, &overlapped, NULL);
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			printf("Sender> WSASend()를 백그라운드로 전송 중...\n");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, NULL);
			// WSASend() 완료된 시점...
			WSAGetOverlappedResult(hSocket, &overlapped, &sendBytes2, FALSE, NULL);
		}
	}
	 
	// send 완료...
	printf("Sender> 송신 데이터 크기: %d (back: %d)bytes.\n", sendBytes, sendBytes2);
	WSACloseEvent(evObj);
	closesocket(hSocket);
	WSACleanup();
	return 0;	
}

void ErrorHandling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}