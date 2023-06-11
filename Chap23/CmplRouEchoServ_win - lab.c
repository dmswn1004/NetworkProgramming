#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#define BUF_SIZE 1024
void CALLBACK ReadCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void CALLBACK WriteCompRoutine(DWORD, DWORD, LPWSAOVERLAPPED, DWORD);
void ErrorHandling(char *message);

typedef struct
{
	SOCKET		hClntSock;
	char		buf[BUF_SIZE];	// App 실제 데이터가 저장되는 공간...
	WSABUF		wsaBuf;			// recv, send 에서 원하는 파라메터 타입...
} PER_IO_DATA, *LPPER_IO_DATA;

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hLisnSock, hRecvSock;	
	SOCKADDR_IN		lisnAdr, recvAdr;
	LPWSAOVERLAPPED lpOvLp;
	DWORD			recvBytes;
	LPPER_IO_DATA	hbInfo;
	int				mode = 1, recvAdrSz, flagInfo = 0;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	// 1. Overlapped IO가 가능한 소켓 생성, recv/send non-blocking 으로 동작
	//    IO 완료 시점 확인 가능 : 1. event 객체 기반, 2. callback - completion routine 등록/호출
	//							: callback 일때. 본 스레드 alertable wait 상태로 가야함.
	hLisnSock = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	// 2. Non-blocking 모드의 IO 및 accept() 수행
	ioctlsocket(hLisnSock, FIONBIO, &mode);
	
	// 3. 서버 주소 설정
	memset(&lisnAdr, 0, sizeof(lisnAdr));
	lisnAdr.sin_family		= AF_INET;
	lisnAdr.sin_addr.s_addr	= htonl(INADDR_ANY);
	lisnAdr.sin_port		= htons(9000);
	if(bind(hLisnSock, (SOCKADDR*) &lisnAdr, sizeof(lisnAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hLisnSock, 5) == SOCKET_ERROR)
		ErrorHandling("listen() error");

	recvAdrSz = sizeof(recvAdr);    
	while(1)
	{
		// 4. CallBack 함수가 호출될 수 있는 alertable wait 상태로 천이
		//    for alertable wait state 천이
		SleepEx(100, TRUE); // APC queue의 callback 메시지 처리

		// 5. 비동기 모드로 accept() 수행.
		hRecvSock =	accept(hLisnSock, (SOCKADDR*)&recvAdr, &recvAdrSz);
		if (hRecvSock == INVALID_SOCKET) {
			if (WSAGetLastError() == WSAEWOULDBLOCK) {
				continue;
			}
			else {
				ErrorHandling("accept() error");
			}
		}

		// 6. 수신 함수 호출... data echo...
		lpOvLp = (LPWSAOVERLAPPED)malloc(sizeof(WSAOVERLAPPED));
		memset(lpOvLp, 0, sizeof(WSAOVERLAPPED));
		
		hbInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA)); // socket, buf 정보 포함...
		hbInfo->hClntSock = hRecvSock;
		hbInfo->wsaBuf.buf = hbInfo->buf;
		hbInfo->wsaBuf.len = BUF_SIZE;

		lpOvLp->hEvent = (HANDLE)hbInfo;

		WSARecv(hRecvSock, &(hbInfo->wsaBuf), 1, &recvBytes, &flagInfo, 
			lpOvLp, ReadCompRoutine);

	}

	closesocket(hRecvSock);
	closesocket(hLisnSock);
	WSACleanup();
	return 0;
}

//
void CALLBACK ReadCompRoutine(
	DWORD dwError, DWORD szRecvBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo	= (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock			= hbInfo->hClntSock;
	LPWSABUF bufInfo		= &(hbInfo->wsaBuf);
	DWORD sentBytes;

	if (szRecvBytes == 0) {
		// 1. 상대 클라이언트가 연결을 종료한 경우 처리
		closesocket(hSock);
		free(lpOverlapped->hEvent); // PER_IO_DATA 구조체, sock, buf 정보
		free(lpOverlapped);
		printf("Read-CR> client disconnected...\n");
	}
	else {
		// 2. 데이터 수신이 완료된 경우, 수신된 데이터 출력 및 클라이언트로 echo back 수행		
		bufInfo->buf[szRecvBytes] = 0; // string...
		printf("Read-CR> rcved data:%s (%d bytes)\n", bufInfo->buf, szRecvBytes);

		// client로 echo back...
		WSASend(hSock, bufInfo, 1, &sentBytes, 0, lpOverlapped, WriteCompRoutine);
	}
}

//
void CALLBACK WriteCompRoutine(
	DWORD dwError, DWORD szSendBytes, LPWSAOVERLAPPED lpOverlapped, DWORD flags)
{
	LPPER_IO_DATA hbInfo	= (LPPER_IO_DATA)(lpOverlapped->hEvent);
	SOCKET hSock			= hbInfo->hClntSock;
	LPWSABUF bufInfo		= &(hbInfo->wsaBuf);
	DWORD recvBytes;
	int flagInfo = 0;

	// 1. 상대 클라이언트로 데이터 송신이 완료된 경우, 새로운 데이터 수신 수행
	
	WSARecv(hSock, bufInfo, 1, &recvBytes, &flagInfo, lpOverlapped, ReadCompRoutine);
}

//
void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
