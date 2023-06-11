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

	// 1. IOCP ���� & ������ ����
	hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0,
					0);
	GetSystemInfo(&sysInfo);
	for (i = 0; i < sysInfo.dwNumberOfProcessors; i++) { // IO ������ ����, �� ������ IO ��û ó��...
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
		
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &addrLen);	// blocking mode ����.	  

		// 2. ���ϰ� IOCP ����.
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

		// 3. WSARecv() ȣ��.
		WSARecv(handleInfo->hClntSock, &(ioInfo->wsaBuf), 1, 
			&recvBytes, &flags,  &(ioInfo->overlapped),
			NULL );

	}
	return 0;
}

// IO ������...IO �ϷḦ ó���ϴ� ������...
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
		// 4. IOCP ���� Ȯ��...
		GetQueuedCompletionStatus(hComPort, &bytesTrans,
		(LPDWORD) & handleInfo,(LPOVERLAPPED) &ioInfo, INFINITE);

		// sock = handleInfo->hClntSock;
		sock = handleInfo->hClntSock;

		if(ioInfo->rwMode == READ)
		{
			// 5. ���� ��� ó��.
			printf("IOThread> msg received.\n");
			
			// 6. socket close ó��
			if (bytesTrans == 0) {
				closesocket(sock);
				free(ioInfo);
				free(handleInfo);
				continue;
			}

			// 7. WSARecv() �Ϸ� ó�� �κп� �ش�...
			// ioInfo->start : recv() ���� �� �� �ƴϸ� �߰����� recv�� ���� ����
			// ioInfo->recvdLen : ������� ���ŵ� ������ ������
			// ioInfo->targetLen : ���� �����ؾ� �ϴ� ������ ��
			// ioInfo->msg[] : ��� ��û �޽����� �����ؼ� �����ϴ� �뵵 calculation(msg)
			//
			// 7.1 ��� ��û �޽����� ���ʷ� ������ ���...
			//   > ���� ���õ� ������ �ʱ�ȭ

			flag = 0;  // ���� recv() �޽��� ������ �Ϸ�Ǿ��� ���� ǥ�� 0: �̿�, 1: �Ϸ�
			if (ioInfo->start == 0) {
				opndCnt = (int)ioInfo->buffer[0]; // �ǿ������� ���� ������ ����.
				ioInfo->recvdLen = bytesTrans;
				ioInfo->targetLen = opndCnt * sizeof(int) + 2;

				// ioInfo->buffer[] -> ioInfo->msg[] ����
				for (int i = 0; i < ioInfo->recvdLen; i++) {
					ioInfo->msg[i] = ioInfo->buffer[i];
				}

				// ���� recv() ���ؼ� ��ü �޽����� ���ŵǾ��� ���� Ȯ��...if no, �߰����� recv() ȣ��
				if (ioInfo->recvdLen < ioInfo->targetLen) {
					ioInfo->start = 1;
				}
				else if (ioInfo->recvdLen == ioInfo->targetLen) {
					printf("IOThread> ���� �Ϸ�: recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
					flag = 1; // ���� �Ϸ�...
				}
			}
			else if (ioInfo->start == 1) {
				// ���� ���ŵ� recv() �Լ��� �ļ����� �޽��� ������ ���� ���̾���.
				// ���ŵ� �����͸� msg[] ����
				for (int i = 0; i < bytesTrans; i++) {
					ioInfo->msg[ioInfo->recvdLen + i] = ioInfo->buffer[i];
				}

				ioInfo->recvdLen += bytesTrans;

				if (ioInfo->recvdLen == ioInfo->targetLen) {
					// ���� �Ϸ�...
					ioInfo->start = 0; 
					flag = 1;
					printf("IOThread> ���� �Ϸ�(�߰��� recv): recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
				}
				else if (ioInfo->recvdLen < ioInfo->targetLen) {
					printf("IOThread> ���� �̿ϼ�. �߰��� recv ȣ�� �ʿ�: recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
				}
				else {
					// ���� ��Ȳ
					printf("IOThread> ���� ��Ȳ: recvdLen:%d, targetLen:%d \n",
						ioInfo->recvdLen, ioInfo->targetLen);
					ioInfo->start = 0;
					flag = 2; // ���� ��Ȳ...
				}
			}
			
			// - ��� ��û �޽����� 1������ �Ϸ����� ���ؼ� �߰����� recv() ȣ�⿡ ���� ���� �κ�...
			//   > ���� ���� �κп� ���� ���Ѿ� ��.
			
			// - ���� �Ϸ� ���ο� ���� 
			//   > �Ϸ�� ���...��� ����(calculation ȣ��)... ����� send() ȣ��
			if (flag == 1) { // ���� �Ϸ�...
				printf("IOThread> recv() done. (start:%d, rcvLen:%d, targetLen:%d)\n",
					ioInfo->start, ioInfo->recvdLen, ioInfo->targetLen);
				// ���.. send...
				result = calculate((int)ioInfo->msg[0], (int*) & (ioInfo->msg[1]), 
					ioInfo->msg[ioInfo->targetLen-1]);

				printf("IOThread> ��� ��� = %d\n", result);

				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				*((int*)(ioInfo->buffer)) = result;
				ioInfo->wsaBuf.len = sizeof(result);
				ioInfo->rwMode = WRITE;

				printf("IOThread> send result = %d\n", result);

				WSASend(sock, &(ioInfo->wsaBuf), 1, NULL,
					0, &(ioInfo->overlapped), NULL);

			}
			else if (flag == 0) { // ���� �̿Ϸ�
				printf("IOThread> recv() needs additional recv() call. (start:%d, rcvLen:%d, targetLen:%d)\n",
					ioInfo->start, ioInfo->recvdLen, ioInfo->targetLen);
				// �ٽ� recv ȣ��...
				memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
				ioInfo->wsaBuf.len = BUF_SIZE;
				ioInfo->wsaBuf.buf = ioInfo->buffer;
				ioInfo->rwMode = READ;
				
				// 3. WSARecv() ȣ��.
				WSARecv(sock, &(ioInfo->wsaBuf), 1,
					NULL, &flags, &(ioInfo->overlapped),
					NULL);
			}
			else {
				printf("IOThread> error case. (start:%d, rcvLen:%d, targetLen:%d)\n",
					ioInfo->start, ioInfo->recvdLen, ioInfo->targetLen);
			}
			//   > �Ϸ���� ���� ���: �߰����� recv() ȣ��
			 
		}
		else if (ioInfo->rwMode == WRITE)
		{
			// WSASend() �Ϸ� ǥ��.
			//puts("message sent!");
			//free(ioInfo);
			printf("IOThread> msg sent. call recv() again.\n");

			// 8. ���� msg ������ ���� WSARecv() ȣ��
			//ioInfo = (LPPER_IO_DATA)malloc(sizeof(PER_IO_DATA));
			memset(&(ioInfo->overlapped), 0, sizeof(OVERLAPPED));
			ioInfo->wsaBuf.len = BUF_SIZE;
			ioInfo->wsaBuf.buf = ioInfo->buffer;
			ioInfo->rwMode = READ;
			ioInfo->start = 0;
			ioInfo->recvdLen = 0;
			ioInfo->targetLen = 0;

			// 3. WSARecv() ȣ��.
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