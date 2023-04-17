
#include <stdio.h>
#include <WinSock2.h>
#define MAX_BUF_SIZE 100
int main(void)
{
	WSADATA		winsockData;
	int			errorNum, errorCode;

	printf("> Ŭ���̾�Ʈ ����. \n");

	// 1. ���� ���̺귯�� �ʱ�ȭ
	errorNum = WSAStartup( MAKEWORD(2,2), &winsockData );
	// 2. ���� ����, param  �������� ����, TCP
	SOCKET clientSocket;
	clientSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	SOCKADDR_IN  serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family			= AF_INET;
	serverAddr.sin_port				= htons(9000);  
	serverAddr.sin_addr.s_addr		= inet_addr("127.0.0.1");

	// ������ �����ϱ�...
	errorNum = connect(clientSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr) );
	printf("> Ŭ���̾�Ʈ�� TCP ���� ��û�Ͽ����ϴ�.\n");

	// �������� send() �Լ� ȣ���� ���ؼ� ������ ������ ����...
	int  index;
	char Buffer[MAX_BUF_SIZE];
	int  returnValue;

	// ������ ������ �غ�...
	for (index = 0; index < MAX_BUF_SIZE; index++) {
		Buffer[index] = index;
	}

	for (index = 0; index < 5; index++ ) 
	{
		returnValue = send(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue == SOCKET_ERROR) {
			printf("<error> send() ���� �� ���� �߻�. code(%d)\n", WSAGetLastError());
		}else if (returnValue > 0) {
			printf("%d> send() �� ���� %d ����Ʈ ����.\n", index+1, returnValue );
		}
		//
		Sleep(1000); 
	}

	// ���� ����
	errorNum = closesocket(clientSocket);
	// ���� ��� ����...
	errorNum = WSACleanup();
}

