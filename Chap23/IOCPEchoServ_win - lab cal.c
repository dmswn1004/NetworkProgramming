#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#define BUF_SIZE	100
#define READ		3
#define	WRITE		5

typedef struct    // socket info
{
	SOCKET			hClntSock; 
	SOCKADDR_IN		clntAdr;
} PER_HANDLE_DATA,	*LPPER_HANDLE_DATA;

typedef struct    // buffer info
{
	OVERLAPPED		overlapped;
	WSABUF			wsaBuf;
	char			buffer[BUF_SIZE];
	int			rwMode;		// READ or WRITE
	char			msg[BUF_SIZE];	// CAL
	int			recvdLen;	// CAL
	int			targetLen;	// CAL
	int			start;		// CAL
} PER_IO_DATA, *LPPER_IO_DATA;

DWORD WINAPI EchoThreadMain(LPVOID CompletionPortIO);
int calculate(int opndCnt, int data[], char op);
void ErrorHandling(char *message);

int main(int argc, char* argv[])
{
	WSADATA			wsaData;
	HANDLE			hComPort;	
	SYSTEM_INFO		sysInfo;
	LPPER_IO_DATA		ioInfo;
	LPPER_HANDLE_DATA	handleInfo;
	SOCKET			hServSock;
	SOCKADDR_IN		servAdr;
	int			recvBytes, i, flags=0;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!"); 

	// 1. IOCP 생성 & 스레드 생성
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,
					0);
	GetSystemInfo(&sysInfo);
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++) { // IO 스레드 생성, 각 소켓의 IO 요청 처리...
		_beginthreadex(NULL, 0, EchoThreadMain, (LPVOID)hComPort, 0, NULL);
	}

	hServSock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr		= htonl(INADDR_ANY);
	servAdr.sin_port		= htons(9000);

	bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr));
	listen(hServSock, 5);
	
	while(1)
	{	
		SOCKET			hClntSock;
		SOCKADDR_IN		clntAdr;		
		int addrLen		= sizeof(clntAdr);
		
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);	// blocking mode 동작.	  

		// 2. 소켓과 IOCP 매핑.
		handleInfo = (LPPER_HANDLE_DATA)malloc(sizeof(PER_HANDLE_DATA));
		handleInfo->hClntSock = hClntSock;
		memcpy(&(handleInfo->clntAdr), &clntAdr, addrLen);

		CreateIoCompletionPort((HANDLE)hClntSock, hComPort, (DWORD)handleInfo, 0);
		
		ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
		memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
		ioInfo->wsaBuf.len	= BUF_SIZE;
		ioInfo->wsaBuf.buf	= ioInfo->buffer;
		ioInfo->rwMode		= READ;
		ioInfo->start		= 0;
		ioInfo->recvdLen	= 0;
		ioInfo->targetLen	= 0;

		// 3. WSARecv() 호출.
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, 
			&recvBytes, &flags,  &(ioInfo->overlapped),
			NULL );

	}
	return 0;
}

// IO 스레드...IO 완료를 처리하는 스레드...
DWORD WINAPI EchoThreadMain(LPVOID pComPort)
{
	HANDLE			hComPort = (HANDLE)pComPort;
	SOCKET			sock;
	DWORD			bytesTrans;
	LPPER_HANDLE_DATA	handleInfo;
	LPPER_IO_DATA		ioInfo;
	DWORD			flags = 0;
	int			opndCnt, flag=0, result;
	
	while(1)
	{ 
		// 4. IOCP 상태 확인...
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
		(LPDWORD) & handleInfo,(LPOVERLAPPED) &ioInfo, INFINITE);

		// sock = handleInfo->hClntSock;
		sock = handleInfo->hClntSock;

		if(ioInfo->rwMode == READ)
		{
			// 5. 수신 모드 처리.
			printf("IOThread> msg received.\n");
			
			// 6. socket close 처리
			if (bytesTrans == 0) {
				closesocket(sock);
				free(ioInfo);
				free(handleInfo);
				continue;
			}

			// 7. WSARecv() 완료 처리 부분에 해당...
			// ioInfo->start : recv() 최초 인 지 아니면 추가적인 recv인 지를 구분
			// ioInfo->recvdLen : 현재까지 수신된 데이터 누적량
			// ioInfo->targetLen : 최종 수신해야 하는 데이터 량
			// ioInfo->msg[] : 계산 요청 메시지를 누적해서 저장하는 용도 calculation(msg)
			//
			// 7.1 계산 요청 메시지를 최초로 수신한 경우...
			//   > 계산과 관련된 변수들 초기화

			flag = 0;  // 현재 recv() 메시지 수신이 완료되었는 지를 표시 0: 미완, 1: 완료
			if (ioInfo->start == 0) {
				opndCnt = (int)ioInfo->buffer[0]; // 피연산자의 갯수 정보를 포함.
				ioInfo->recvdLen = bytesTrans;
				ioInfo->targetLen = opndCnt * sizeof(int) + 2;

				// ioInfo->buffer[] -> ioInfo->msg[] 복사
				for (int i = 0; i < ioInfo->recvdLen; i++) {
					ioInfo->msg[i] = ioInfo->buffer[i];
				}

				// 현재 recv() 통해서 전체 메시지가 수신되었는 지를 확인...if no, 추가적인 recv() 호출
				if (ioInfo->recvdLen < ioInfo->targetLen) {
					ioInfo->start = 1;
				}
				else if (ioInfo->recvdLen == ioInfo->targetLen) {
					printf("IOThread> 수신 완료: recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
					flag = 1; // 수신 완료...
				}
			}
			else if (ioInfo->start == 1) {
				// 현재 수신된 recv() 함수는 후속적인 메시지 수신을 위한 것이었음.
				// 수신된 데이터를 msg[] 저장
				for (int i = 0; i < bytesTrans; i++) {
					ioInfo->msg[ioInfo->recvdLen + i] = ioInfo->buffer[i];
				}

				ioInfo->recvdLen += bytesTrans;

				if (ioInfo->recvdLen == ioInfo->targetLen) {
					// 수신 완료...
					ioInfo->start = 0; 
					flag = 1;
					printf("IOThread> 수신 완료(추가적 recv): recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
				}
				else if (ioInfo->recvdLen < ioInfo->targetLen) {
					printf("IOThread> 수신 미완성. 추가적 recv 호출 필요: recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
				}
				else {
					// 오류 상황
					printf("IOThread> 오류 상황: recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
					ioInfo->start = 0;
					flag = 2; // 오류 상황...
				}
			}
			
			// - 계산 요청 메시지를 1번만에 완료하지 못해서 추가적인 recv() 호출에 따른 수신 부분...
			//   > 기존 계산된 부분에 누적 시켜야 함.
			
			// - 수신 완료 여부에 따라 
			//   > 완료된 경우...계산 수행(calculation 호출)... 결과를 send() 호출
			if (flag == 1) { // 수신 완료...
				printf("IOThread> recv() done. (start:%d, rcvLen:%d, targetLen:%d)\n",
					ioInfo->start, ioInfo->recvdLen, ioInfo->targetLen);
				// 계산.. send...
				result = calculate((int)ioInfo->msg[0], (int*) & (ioInfo->msg[1]), 
					ioInfo->msg[ioInfo->targetLen-1]);

				printf("IOThread> 계산 결과 = %d\n", result);

				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				*((int*)(ioInfo->buffer)) = result;
				ioInfo->wsaBuf.len = sizeof(result);
				ioInfo->rwMode = WRITE;

				printf("IOThread> send result = %d\n", result);

				WSASend(sock, &(ioInfo->wsaBuf), 1, NULL,
					0, &(ioInfo->overlapped), NULL);

			}
			else if (flag == 0) { // 수신 미완료
				printf("IOThread> recv() needs additional recv() call. (start:%d, rcvLen:%d, targetLen:%d)\n",
					ioInfo->start, ioInfo->recvdLen, ioInfo->targetLen);
				// 다시 recv 호출...
				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				ioInfo->wsaBuf.len = BUF_SIZE;
				ioInfo->wsaBuf.buf = ioInfo->buffer;
				ioInfo->rwMode = READ;
				
				// 3. WSARecv() 호출.
				WSARecv(sock, &(ioInfo->wsaBuf), 1,
					NULL, &flags, &(ioInfo->overlapped),
					NULL);
			}
			else {
				printf("IOThread> error case. (start:%d, rcvLen:%d, targetLen:%d)\n",
					ioInfo->start, ioInfo->recvdLen, ioInfo->targetLen);
			}
			//   > 완료되지 않은 경우: 추가적인 recv() 호출
			 
		}
		else if (ioInfo->rwMode == WRITE)
		{
			// WSASend() 완료 표시.
			//puts("message sent!");
			//free(ioInfo);
			printf("IOThread> msg sent. call recv() again.\n");

			// 8. 다음 msg 수신을 위해 WSARecv() 호출
			//ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			ioInfo->start = 0;
			ioInfo->recvdLen = 0;
			ioInfo->targetLen = 0;

			// 3. WSARecv() 호출.
			WSARecv(sock, &(ioInfo->wsaBuf), 1,
				NULL, &flags, &(ioInfo->overlapped),
				NULL);

		}
	}
	return 0;
}


int calculate(int opndCnt, int data[], char op)
{
	int result = data[0];
	switch (op) {
	case '+':
		for (int i = 1; i < opndCnt; i++) {
			result += data[i];
		}
		break;
	case '-':
		for (int i = 1; i < opndCnt; i++) {
			result -= data[i];
		}
		break;
	case '*':
		for (int i = 1; i < opndCnt; i++) {
			result *= data[i];
		}
		break;
	}

	return result;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}