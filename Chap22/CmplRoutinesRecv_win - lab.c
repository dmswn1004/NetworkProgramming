#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#define LOOP_CNT	5
#define BUF_SIZE	1024

void CALLBACK CompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char *message);

WSABUF	dataBuf;
char	buf[BUF_SIZE];
int		recvBytes=0;

int main(int argc, char* argv[])
{
	WSADATA			wsaData;
	SOCKET			hLisnSock, hRecvSock;	
	SOCKADDR_IN		lisnAdr, recvAdr;
	WSAOVERLAPPED	overlapped;
	WSAEVENT		evObj;
	int				idx, recvAdrSz, flags=0;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family			= AF_INET;
	lisnAdr.sin_addr.s_addr		= inet_addr("127.0.0.1");
	lisnAdr.sin_port			= htons(9000);

	if(bind(hLisnSock, (SOCKADDR*) &lisnAdr, sizeof(lisnAdr)) == SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAdrSz = sizeof(recvAdr);    
	hRecvSock = accept(hLisnSock, (SOCKADDR*)&recvAdr,&recvAdrSz);
	if(hRecvSock == INVALID_SOCKET)
		ErrorHandling("accept() error");

	int ret;
	for (int i = 0; i < LOOP_CNT; i++) {

		printf("Rcver> try to receive. (%d)\n", i);

		dataBuf.buf = buf;
		dataBuf.len = BUF_SIZE;
		memset(&overlapped, 0, sizeof(overlapped));

		evObj = WSACreateEvent(); // dummy event 객체 생성...

		printf("Rcver> call WSARecv().\n");
		ret = WSARecv(	hRecvSock, &dataBuf, 1, &recvBytes, &flags, 
						&overlapped, CompRoutine);
		if (ret == SOCKET_ERROR) {
			if (WSAGetLastError() == WSA_IO_PENDING) {
				printf("Rcver> background로 wsarecv() 실행 중.\n");
			}
		}

		// Alertable wait 상태로 진입 필요...
		idx = WSAWaitForMultipleEvents(1, &evObj, FALSE, WSA_INFINITE, TRUE );
		if (idx == WAIT_IO_COMPLETION) {
			printf("Rcver> overlapped compRoutine 실행 완료.\n");
		}
		else {
			ErrorHandling("WSARecv() error");
		}

	}

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

void CALLBACK CompRoutine(DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	// ---
	if (dwError != 0) {
		ErrorHandling("CompRoutine error");
	}
	else {
		recvBytes = szRecvBytes;
		printf("CompRt> receiving msg:%s (%d bytes)\n", buf, szRecvBytes);
	}
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
