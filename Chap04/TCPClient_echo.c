#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main()
{
	WSADATA			wsaData;
	SOCKET			hSocket;
	char			message[BUF_SIZE];
	int				strLen, ret;
	SOCKADDR_IN		servAdr;

	if( WSAStartup( MAKEWORD(2, 2), &wsaData ) != 0 )
		ErrorHandling("WSAStartup() error!"); 

	// 1. socket 생성
	hSocket = socket( PF_INET, SOCK_STREAM, 0 );   
	if( hSocket == INVALID_SOCKET )
		ErrorHandling("socket() error");
	
	// 2. connect로 서버와 연결 설정 요청
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family = AF_INET; // IPv4
	servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAdr.sin_port = htons(9000); // Big endian = network byte ordering

	printf("Client> try to connect with server.\n");
	ret = connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr));
	if(ret == SOCKET_ERROR){
		ErrorHandling("connect() error");
	}

	printf("Client> connected...\n");
	// Sleep(10000);

	// client APP 프로토콜 구현...(send/recv)...
	// 1~3까지 무한 반복...
	int str_len, total_len, rcv_sum;
	while (1)
	{
		// 0. 서버와 주고 받고 약속 필요 (주고 받는 데이터 크기)
		// 1. 사용자로부터 전송할 데이터 입력받기...
		printf("Clint> Input message (Q/q to quit): ");
		scanf("%s", message); // abc  -> abc0
		if (!strcmp(message, "q") || !strcmp(message, "Q")) {
			break;
		}
		// 2. 입력된 데이터를 서버로 전송 send()
		str_len = send(hSocket, message, strlen(message), 0); // send 한 byte 수 리턴
		if(str_len == SOCKET_ERROR){
			printf("Client> send() error\n");
		}
		printf("Client> sent message(%s) to server(%d bytes)\n",message, str_len);

		// 3. 서버가 echo back한 데이터를 수신하기. recv()
		strLen = recv(hSocket, message, BUF_SIZE - 1, 0);
		message[strLen] = 0;
		printf("Client> message from server: %s\n", message);

		//total_len = str_len;
		//rcv_sum = 0;
		//while (rcv_sum < total_len){
		//	str_len = recv(hSocket, &message[rcv_sum], BUF_SIZE-1, 0); // recv 리턴 값 : 수신한 바이트 수 / 0을 리턴한 경우 상대방이 close함(기억하기)
		//	rcv_sum += str_len;
		//	printf("Client-TCP> recv %d bytes. (rcv_sum : %d , total_len : %d)\n ", str_len, rcv_sum, total_len);
		//}
	}
	closesocket( hSocket );
	WSACleanup();
	return 0;
}

void ErrorHandling( char *message )
{
	printf("[ERROR] %s \n", message );	
	exit(1);
}