#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#define MAX_PACKET_SIZE 120
void ErrorHandling(char *message);
int calculate(int opndCnt, int data[], char op);

int main()
{
	WSADATA		wsaData;
	SOCKET		hServSock, hClntSock;
	int		i=0;
	SOCKADDR_IN 	servAdr, clntAdr;
	int		clntAdrSize;
	char		opndCnt;
	char		data[MAX_PACKET_SIZE];
	int		backlog = 2, ret, flag;
	int		rcvTarget, rcvSum = 0, result;

	if( WSAStartup(MAKEWORD(2, 2), &wsaData) !=0 )
		ErrorHandling("WSAStartup() error!"); 
	
	// 1. socket 생성하기.
	hServSock = socket(PF_INET, SOCK_DGRAM, 0);  // UDP를 사용. 
	if(hServSock==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr		= inet_addr("127.0.0.1"); // big endian.
								  //htonl( INADDR_ANY ); // Big endian
	servAdr.sin_port		= htons( 9000 );

	// 2. bind 서버 소켓에 주소 설정하기
	if( bind(hServSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR )
		ErrorHandling("bind() error");
	
#if 0
	// 3. listen : 클라이언트의 연결 요청 수신 준비	
	ret = listen(hServSock, backlog);
	if (ret == SOCKET_ERROR) {
		ErrorHandling("listen() error");
	}
#endif
	// 서버이므로 여러개의 client 연결 요청을 수락 및 서비스 제공(recv/send)
	while (1)
	{
		// 4. accept : 클라이언트 연결 설정을 수락.
		// 새로운 데이터 전송용 소켓 생성.
		clntAdrSize = sizeof(clntAdr);
		
#if 0
		printf("Server> wait client...\n");
		hClntSock = accept(hServSock,(SOCKADDR*)&clntAdr, &clntAdrSize);
		if (hClntSock == INVALID_SOCKET) {
			ErrorHandling("accept() error");
		}
		printf("Server> a client(IP:%s, Port:%d) is connedted.\n",
			inet_ntoa(clntAdr.sin_addr),
			ntohs(clntAdr.sin_port)
		);
#endif
		//flag = 1;
		// server APP 프로토콜 구현하기...기능: 계산기 구현
		//while (flag)
		//{
		printf("Server> 클라이언트의 계산 요청 대기.\n");
		// 0. 수신 시 상대 client의 종료 확인 및 loop에서 탈출...
		// 1. 1 바이트 읽어서 피연산자 수 및 추가로 읽어야 할 데이터량 계산
		ret = recvfrom(	hServSock, data, MAX_PACKET_SIZE, 0,
						(SOCKADDR*)&clntAdr, &clntAdrSize
					  );
		if (ret <= 0) {
			printf("Server> recv() error\n");
			break;
		}

		// 2. 나머지 수신해야할 데이터 수신...loop	
#if 0
		rcvSum = 0;
		rcvTarget = opndCnt * sizeof(int) + 1;
		while (rcvSum < rcvTarget) {
			ret = recv(hClntSock, &data[rcvSum], rcvTarget-rcvSum, 0);
			if (ret <= 0) {
				flag = 0;
				break;
			}
			rcvSum += ret;
		}
#endif
		//if (flag == 1)
		//{
		// 3. 계산
		
		result = calculate((int)data[0], (int*)&data[1], data[ret-1]);
		printf("Server> 계산 결과 = %d\n", result);
		// 4. 결과 client로 전송
		ret = sendto(	hServSock, (char*)&result, sizeof(result), 0,
						(SOCKADDR*)&clntAdr, sizeof(clntAdr)
					);
		if (ret == SOCKET_ERROR) {
			printf("Server> send() error\n");
			break;
		}
		//}
		//}
#if 0
		printf("Server> 클라이언트(IP:%s, Port:%d)와 연결 종료.\n",
			inet_ntoa(clntAdr.sin_addr), ntohs(clntAdr.sin_port));
		closesocket(hClntSock);
#endif
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

int calculate(int opndCnt, int data[], char op)
{
	int result = data[0];
	switch (op) {
	case '+':
		for (int i = 1; i < opndCnt; i++) {
			result = result + data[i];
		}
		break;
	case '-':
		for (int i = 1; i < opndCnt; i++) {
			result = result - data[i];
		}
		break;
	case '*':
		for (int i = 1; i < opndCnt; i++) {
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
