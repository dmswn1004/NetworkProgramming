#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void ErrorHandling(char *message);
int calculation(int opndCnt, int data[], char op);
DWORD WINAPI ProcessClient(LPVOID arg);

#define MAX_PACKET_SIZE  120

int main(void)
{
	WSADATA wsaData;
	SOCKET hServSock, hClntSock;
	SOCKADDR_IN servAdr, clntAdr;
	int clntAdrSize;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	hServSock = socket(PF_INET, SOCK_STREAM, 0);  

	if(hServSock == INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=htonl(INADDR_ANY);
	servAdr.sin_port=htons(9000);

	if(bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	
	// Ŭ���̾�Ʈ���� ������ �غ� �Ϸ�...
	listen(hServSock, 3);

	HANDLE hThread;
	DWORD ThreadId;

	while (1) {
		printf("Server> Ŭ���̾�Ʈ ���� ��û �����.\n");

		clntAdrSize = sizeof(clntAdr);
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSize);
		if (hClntSock == -1) {
			printf("<ERROR> accept ���� ����.\n");
		}
		else {
			printf("Server> client(IP:%s, Port:%d) �����.\n",
				inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));
		}

		// ���ο� ������ ����...
		hThread =	_beginthreadex(
						NULL,	// ���ȼ���...
						0,		// ���� ũ��
						ProcessClient,		// �����尡 ������ �Լ���
						(LPVOID)hClntSock,
						0, 
						&ThreadId
					);

		if (hThread == NULL) {
			printf("<ERROR> ������ ���� ����.\n");
		}
		else {
			CloseHandle(hThread);
		}
		// ��� ó���ϴ� while loop -> ProcessClient �Լ��� ����...
		
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	int flag = 1;
	int rcvSum, rcvTotal, ret, result;
	char opndCnt, msg[MAX_PACKET_SIZE];
	SOCKET hClntSock;
	hClntSock = (SOCKET)arg;

	printf("<THREAD> ���ο� ������ ����.\n");
	while (flag) {
		printf("THREAD> ��� ��û ��� ��.\n");
		recv(hClntSock, &opndCnt, sizeof(opndCnt), 0);
		printf("THREAD> �ǿ����� �� = %d.\n", opndCnt);

		rcvSum = 0; // ���� ����ġ.
		rcvTotal = opndCnt * sizeof(int) + 1; // ���� ��ǥġ.
		while (rcvSum < rcvTotal) {
			ret = recv(hClntSock, &msg[rcvSum], rcvTotal - rcvSum, 0);
			if (ret <= 0) {
				printf("<ERROR> recv() ����.\n");
				flag = 0;
				break;
			}
			else {
				rcvSum = rcvSum + ret;
				printf("THREAD> recv %d bytes. sum=%d, total=%d\n", ret, rcvSum, rcvTotal);
			}
		}
		if (flag == 1) {
			// 2. ��� ���� 				
			result = calculation((int)opndCnt, (int*)msg, msg[rcvTotal - 1]);
			printf("THREAD> ��� ��� = %d.\n", result);
			// 3. send(result) to client ����
			send(hClntSock, (char*)&result, sizeof(result), 0);
			printf("THREAD> ��� ��� client�� ����.\n");
		}
	}
	printf("THREAD> close socket with client.\n");
	closesocket(hClntSock);
}

int calculation(int opndCnt, int data[], char op)
{
	int result, i;
	result = data[0];
	switch (op) {
	case '+':
		for (i = 1; i < opndCnt; i++) {
			result = result + data[i];
		}
		break;
	case '-':
		for (i = 1; i < opndCnt; i++) {
			result = result - data[i];
		}
		break;
	case '*':
		for (i = 1; i < opndCnt; i++) {
			result = result * data[i];
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
