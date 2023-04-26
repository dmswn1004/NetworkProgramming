#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main()
{
	WSADATA		wsaData;
	SOCKET		hServSock, hClntSock;
	char		message[BUF_SIZE];
	int			strLen, i=0;
	SOCKADDR_IN servAdr, clntAdr;
	int			clntAdrSize, ret;

	if( WSAStartup(MAKEWORD(2, 2), &wsaData) !=0 )
		ErrorHandling("WSAStartup() error!"); 
	
	// 1. 소켓 생성 (연결 수신 소켓 : 리스닝 소켓, 서버 소켓)
	hServSock=socket(PF_INET, SOCK_STREAM, 0);   // TCP 소켓 생성
	if(hServSock==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	// 주소 설정
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family			= AF_INET;
	servAdr.sin_addr.s_addr			= inet_addr("127.0.0.1"); // 32bits 숫자, 네트워크 바이트 순서, Big endian으로 변경해주는 함수 사용
								// ntohl(inet_addr("127.0.0.1")); 네트워크에서 호스트로 (big endian->little endian)으로 변경
								// htonl( INADDR_ANY );  // Big endian
	servAdr.sin_port			= htons( 9000 ); // big endian으로 변경해주는 함수 사용
								// = ntohs(htons(0x123)); 네트워크에서 호스트로 (big endian->little endian)으로 변경

	// 2. bind() : 서버 소켓에 주소 설정하기
	if( bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR )
		ErrorHandling("bind() error");
	
	// 3. listen() : 클라이언트 연결 수용 준비 완료
	int backLog = 2; // 연결 요청 대기 큐의 크기 / 초과시 error 발생
	ret = listen(hServSock, backLog);
	if (ret == SOCKET_ERROR) {
		ErrorHandling("listen() error");
	}

	// 4. accept() : 클라이언트의 연결 수락, (중요) 새로운 데이터 전송용 소켓 생성
	// hClntSock : 데이터 전송 소켓, 연결된 client와 1:1 연결

	// 서버이므로 여러개의 client 연결 요청을 수락 및 서비스 제공(recv/send)
	while(1)
	{
		// 4. accept() : 클라이언트의 연결 수락, (중요) 새로운 데이터 전송용 소켓 생성
		clntAdrSize = sizeof(clntAdr);
		printf("Server> waiting client...\n");
		hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSize);
		if(hClntSock == SOCKET_ERROR) {
			ErrorHandling("accept() error");
		}
		printf("Server> client(IP:%s, Ports:%d) is connected\n", inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));

		// 서버 App 프로토콜 구현하기... 기능: echo back
		while( (strLen = recv(hClntSock,message, BUF_SIZE-1, 0)) > 0 )
		{
			// 1~2까지 무한 반복...
			// 1. client가 송신한 데이터를 수신하기...recv();
			// 1-1. recv() 리턴 값이 0 (상대방 socketclose)

			// 2. echo back...구현...받은 데이터를 client로 송신하기...send();
			send(hClntSock, message, strlen, 0);
			message[strLen] = 0;
			printf("Server> echo back message(%s) to client.\n", message);
		}
		printf("Server> client(IP:%s, Port:%d) connect closed.\n", inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));
		closesocket(hClntSock); // 클라이언트 소켓: 클라이언트의 데이터 송수신
	}
	closesocket(hServSock); // 서버 소켓: 연결 청취(listening) 소켓
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
