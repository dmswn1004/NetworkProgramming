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
	SOCKADDR_IN		servAdr, clntAdr;
	int				clntAdrSize;

	if( WSAStartup( MAKEWORD(2, 2), &wsaData ) != 0 )
		ErrorHandling("WSAStartup() error!"); 

	hSocket = socket( PF_INET, SOCK_DGRAM, 0 );  // UDP를 사용.. 
	if( hSocket == INVALID_SOCKET )
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family			= AF_INET;
	servAdr.sin_port			= htons(9000); // Big endian = network byte ordering
	servAdr.sin_addr.s_addr		= inet_addr("127.0.0.1");

	int ret;
#if 0
	printf("Client> try to connect with server.\n");
	
	ret = connect(	hSocket, 
				(SOCKADDR*)&servAdr, 
				sizeof(servAdr)
			);
	if (ret == SOCKET_ERROR) {
		ErrorHandling("connect() error");
	}
	printf("Client> connected...\n");
#endif

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
		printf("Client> 피연산자 수 입력: ");
		scanf("%d", &opndCnt);
		data[0] = (char)opndCnt;

		// 2. 입력된 갯수만큼 피연산자 입력받기(data encapsulation)
		for (int i = 0; i < opndCnt; i++) {
			printf("- 피연산자: ");
			scanf("%d", (int*)&data[1+i*sizeof(int)]);
		}

		rewind(stdin); // scanf %c로 읽기 전에 키보드 버퍼 클리어 필요.
		// 3. 연산자 입력받기(data encapsulation)
		printf("Client> 연산자 입력: ");
		scanf("%c", &data[1+opndCnt*sizeof(int)]);


		// 4. 서버로 만들어진 data 송신	
		//ret = send(hSocket, data, 2 + opndCnt * sizeof(int), 0);
		ret = sendto(	hSocket, data, 2 + opndCnt * sizeof(int), 0, 
						(SOCKADDR*)&servAdr, sizeof(servAdr));

		if (ret == SOCKET_ERROR) {
			ErrorHandling("ERROR> send() error");
		}
		printf("Client> 서버에게 %d bytes 전송.\n", ret);

		// 5. 서버의 계산 결과(4bytes) 수신 및 화면에 출력
#if 0
		rcvSum = 0;
		rcvTarget = sizeof(result);
		while (rcvSum < rcvTarget) {
			ret = recv(hSocket, (char*)&data[rcvSum], rcvTarget-rcvSum, 0);
			if (ret <= 0) {
				flag = 0;
				break;
			}
			rcvSum += ret;
		}
#endif
		clntAdrSize = sizeof(clntAdr);
		recvfrom(	hSocket, (char*)&result, sizeof(result), 0,
					(SOCKADDR*)&clntAdr, &clntAdrSize
				);

		//result = *((int*)data);
		printf("Client> 서버가 전달한 결과 = %d\n", result);
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
