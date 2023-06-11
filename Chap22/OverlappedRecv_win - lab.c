#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#define BUF_SIZE 1024

void ErrorHandling(char *message);

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hLisnSock, hRecvSock;
	SOCKADDR_IN		lisnAdr, recvAdr;
	int				recvAdrSz;

	WSABUF			dataBuf;
	WSAEVENT		evObj;
	WSAOVERLAPPED	overlapped;

	char	buf[BUF_SIZE];
	int		recvBytes = 0, recvBytes2 = 0, flags = 0;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family = AF_INET;
	lisnAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
	lisnAdr.sin_port = htons(9000);

	if(bind(hLisnSock, (SOCKADDR*)&lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");

	if(listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAdrSz = sizeof(recvAdr);    
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);
	printf("Recver> new client is accepted.\n");

	int ret;

	dataBuf.buf = buf;
	dataBuf.len = BUF_SIZE;

	evObj = WSACreateEvent(); // manual reset...
	memset(&overlapped, 0, sizeof(overlapped));
	overlapped.hEvent = evObj;

	// overlapped IO receiver 호출...
	ret = WSARecv(hRecvSock, &dataBuf, 1, &recvBytes, 
					&flags, &overlapped, NULL);
	
	if (ret == SOCKET_ERROR) {
		if (WSAGetLastError() == WSA_IO_PENDING) {
			printf("Recver> WSARecv()를 백그라운드로 수신 중.\n");
			WSAWaitForMultipleEvents(1, &evObj, TRUE, WSA_INFINITE, FALSE);
			// 데이터 수신 event 발생...
			WSAGetOverlappedResult(hRecvSock, &overlapped, &recvBytes2, 
				FALSE, NULL);
			printf("Recver> 백그라운드 데이터 수신. %d bytes\n", recvBytes2);
		}
	}

	printf("Recver> WSARecv() 수신 완료. msg = %s (%d bytes)..\n", 
		buf, recvBytes);
	WSACloseEvent(evObj);

	// ---

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}