// ���� ���α׷�.
#include <stdio.h>
#include <WinSock2.h>
#define  MAX_BUF_SIZE  1000
int main(void)
{
	WSADATA   winsockData;
	printf("> ���� ���α׷� ����.\n");

	// 1. ���� �ʱ�ȭ...���� ���̺귯���� ����...
	WSAStartup( MAKEWORD(2,2), &winsockData );
	
	// 2. ������ ���� �ϱ�...
	SOCKET serverSocket;
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN   serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_port			= htons(9000); 
	serverAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");

	// 3. bind() �Լ� ȣ��...
	// - ������ ���� IP, port ����...
	bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	// 4. listen() �Լ� ȣ��...
	printf("> listen �Լ��� ȣ���մϴ�.\n");
	int backLog = 5; 
	listen(serverSocket, backLog );

	// 5. accept �Լ� ȣ��...
	SOCKET			clientSocket;
	SOCKADDR_IN		clientAddr;
	int				addrLen;
	addrLen = sizeof(clientAddr);

	printf("> accept �Լ��� ȣ���Ͽ� Ŭ���̾�Ʈ ���� ��û�� ����մϴ�.\n");
	clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrLen);

	printf("> Ŭ���̾�Ʈ(IP:%s, Port:%d)�� ����Ǿ����ϴ�.\n",
		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	// recv() �Լ��� ���� ��� tcp client�κ��� ������ ����
	char Buffer[MAX_BUF_SIZE];
	int  returnValue, stopFlag=0, index=1;
	
	while (stopFlag == 0) {

		returnValue = recv(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue > 0) // ������ �����͸� ������ ���...
		{
			printf("%d> %d ����Ʈ �����͸� ����.\n", index, returnValue);
			index++;
		}else if (returnValue == SOCKET_ERROR) {
			printf("<error> recv() �Լ� ���� �� ���� �߻�, code(%d)\n", WSAGetLastError());
		}else if (returnValue == 0) {
			// ���� ���� ����...
			printf("> client�� socket�� ����\n");
			stopFlag = 1;
		}
		//Sleep(1000);
	}

	// ������ ���� ��� ����...
	closesocket(serverSocket);
	
	// ���� ��� ����..
	WSACleanup();
	return 0;
}