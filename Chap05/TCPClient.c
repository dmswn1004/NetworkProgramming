#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#define MAX_PACKET_SIZE 120
void ErrorHandling(char *message);

int main()
{
	WSADATA			wsaData;
	SOCKET			hSocket;
	int				strlen, ret;
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
	servAdr.sin_port = htons(9000);  // Big endian = network byte ordering

	printf("Client> try to connect with server.\n");
	ret = connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr));
	if(ret == SOCKET_ERROR){
		ErrorHandling("connect() error");
	}
	printf("Client> connected...\n");
	// Sleep(10000);

	// client APP 프로토콜 구현...(send/recv)...
	// 1~3까지 무한 반복...
	int rcvSum = 0, rcvTarget, result;
	int flag = 1, opndCnt; 
	char data[MAX_PACKET_SIZE];
	while (flag)
	{
		// send( data, ) char data[1024] -> msg 구성
		// 계산기 APP 계층 Client 프로토콜을 작성하기...(server/client)
		// 1. 피연산자 수 입력받기(data encapsulation)
		printf("Client> Enter the number of operands : ");
		scanf("%d", &opndCnt);
		data[0] = (char)opndCnt;

		// 2. 입력된 갯수만큼 피연산자 입력받기(data encapsulation)
		for (int i = 0; i < opndCnt; i++) {
			printf("- operand : ");
			scanf("%d", (int*)&data[1 + i * sizeof(int)]);
		}

		rewind(stdin); // scanf %c로 읽기 전에 키보드 버퍼 클리어 필요

		// 3. 연산자 입력받기 (msg encapsulation)
		// 3. 연산자 입력받기(data encapsulation)
		printf("Client> Operator input : ");
		scanf("%c", &data[1 + opndCnt * sizeof(int)]);
		
		// 4. 서버로 만들어진 data 송신	
		ret = send(hSocket, data, 2 + opndCnt * sizeof(int), 0);
		if (ret == SOCKET_ERROR) {
			ErrorHandling("ERROR> send() error");
		}
		printf("Client> Send %d bytes to server.\n", ret);
		
		// 5. 서버의 계산 결과(4bytes) 수신 및 화면에 출력
		rcvSum = 0;
		rcvTarget = sizeof(result);
		while(rcvSum < rcvTarget)
		{
			ret = recv(hSocket, (char*)&data[rcvSum], rcvTarget-rcvSum, 0);
			rcvSum += ret;
			if(ret <= 0){
				flag = 0;
				break;
			}
			result = *((int*)data);
			printf("Client> Result delivered by server = %d\n", result);
		}
		// Sleep(20000);
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