// 서버 프로그램.
#include <stdio.h>
#include <WinSock2.h>
#define  MAX_BUF_SIZE  1000
int main(void)
{
	WSADATA   winsockData;
	printf("> 서버 프로그램 실행.\n");

	// 1. 소켓 초기화...소켓 라이브러리를 연결...
	WSAStartup( MAKEWORD(2,2), &winsockData );
	
	// 2. 소켓을 생성 하기...
	SOCKET serverSocket;
	serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	SOCKADDR_IN   serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family		= AF_INET;
	serverAddr.sin_port			= htons(9000); 
	serverAddr.sin_addr.s_addr	= inet_addr("127.0.0.1");

	// 3. bind() 함수 호출...
	// - 생성된 소켓 IP, port 설정...
	bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));

	// 4. listen() 함수 호출...
	printf("> listen 함수를 호출합니다.\n");
	int backLog = 5; 
	listen(serverSocket, backLog );

	// 5. accept 함수 호출...
	SOCKET			clientSocket;
	SOCKADDR_IN		clientAddr;
	int				addrLen;
	addrLen = sizeof(clientAddr);

	printf("> accept 함수를 호출하여 클라이언트 연결 요청을 대기합니다.\n");
	clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &addrLen);

	printf("> 클라이언트(IP:%s, Port:%d)가 연결되었습니다.\n",
		inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

	// recv() 함수를 통해 상대 tcp client로부터 데이터 수신
	char Buffer[MAX_BUF_SIZE];
	int  returnValue, stopFlag=0, index=1;
	
	while (stopFlag == 0) 
	{
		returnValue = recv(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue > 0) // 일정한 데이터를 수신한 경우...
		{
			printf("%d> %d 바이트 데이터를 수신.\n", index, returnValue);
			index++;
		}else if (returnValue == SOCKET_ERROR) {
			printf("<error> recv() 함수 실행 중 오류 발생, code(%d)\n", WSAGetLastError());
		}else if (returnValue == 0) {
			// 상대방 연결 종료...
			printf("> client가 socket을 종료\n");
			stopFlag = 1;
		}
		//Sleep(1000);
	}

	// 소켓을 통한 통신 종료...
	closesocket(serverSocket);
	
	// 소켓 사용 종료..
	WSACleanup();
	return 0;
}