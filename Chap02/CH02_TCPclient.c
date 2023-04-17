
#include <stdio.h>
#include <WinSock2.h>
#define MAX_BUF_SIZE 100
int main(void)
{
	WSADATA		winsockData;
	int			errorNum, errorCode;

	printf("> 클라이언트 실행. \n");

	// 1. 소켓 라이브러리 초기화
	errorNum = WSAStartup( MAKEWORD(2,2), &winsockData );
	// 2. 소켓 생성, param  프로토콜 지정, TCP
	SOCKET clientSocket;
	clientSocket = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );

	SOCKADDR_IN  serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family			= AF_INET;
	serverAddr.sin_port				= htons(9000);  
	serverAddr.sin_addr.s_addr		= inet_addr("127.0.0.1");

	// 서버와 연결하기...
	errorNum = connect(clientSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr) );
	printf("> 클라이언트가 TCP 연결 요청하였습니다.\n");

	// 여러개의 send() 함수 호출을 통해서 서버로 데이터 전송...
	int  index;
	char Buffer[MAX_BUF_SIZE];
	int  returnValue;

	// 전송할 데이터 준비...
	for (index = 0; index < MAX_BUF_SIZE; index++) {
		Buffer[index] = index;
	}

	for (index = 0; index < 5; index++ ) 
	{
		returnValue = send(clientSocket, Buffer, MAX_BUF_SIZE, 0);
		if (returnValue == SOCKET_ERROR) {
			printf("<error> send() 수행 중 오류 발생. code(%d)\n", WSAGetLastError());
		}else if (returnValue > 0) {
			printf("%d> send() 를 통해 %d 바이트 전송.\n", index+1, returnValue );
		}
		//
		Sleep(1000); 
	}

	// 소켓 종료
	errorNum = closesocket(clientSocket);
	// 소켓 사용 종료...
	errorNum = WSACleanup();
}

