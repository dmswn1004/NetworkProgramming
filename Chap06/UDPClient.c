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
	char			message[BUF_SIZE];
	int				strLen, ret, clntAdrSize;
	SOCKADDR_IN		servAdr, clntAdr;

	if( WSAStartup( MAKEWORD(2, 2), &wsaData ) != 0 )
		ErrorHandling("WSAStartup() error!"); 

	// 1. socket 생성
	hSocket = socket( PF_INET, SOCK_DGRAM, 0);   
	if( hSocket == INVALID_SOCKET )
		ErrorHandling("socket() error");
	
	// 2. connect로 서버와 연결 설정 요청
	// memset(servAdr, 0, sizeof(servAdr)); 
	// servAdr.sin_familly = AR_INET; // IPv4
	// servAdr.sin_port = htons(9000); // network byte ordering = big endian
	// servAdr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// ret = connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr));
	// if(ret == SOCKET_ERROR){
	// 	ErrorHandling("connect() error");
	// }
	// printf("Client> connedcted with server.\n");
	// Sleep(10000);

	// App 프로토콜 구현...
	int rcvSum = 0, rcvTarget, result;
	int opndCut;
	int flag = 1; // true
	char data[MAX_PACKET_SIZE];
	while (flag){
		// send( message,  ) char message[1024] -> msg 구성
		// 계산기 App 계층 Client 프로토콜을 작성
		// 1. 피연산자 수 입력받기 (msg encapsulation)
		printf("Client> 피연산자 수 입력 : ");
		scanf("%d", &opndCut);
		data[0] = (char)opndCut;
		// 2. 입력된 갯수만큼 피연산자 입력받기 (msg encapsulation)
		for(int i = 0; i < opndCut; i++){
			printf(" - 피연산자 : ");
			scanf("%d", (int*)&data[1+i*sizeof(int)]);
		}
		rewind(stdin); // scanf %c로 읽기 전에 키보드 버퍼 클리어 필요
		// 3. 연산자 입력받기 (msg encapsulation)
		printf("Client> 연산자를 입력 : ");
		scanf("%c", &data[1+opndCut*sizeof(int)]);
		
		// 4. 서버로 만들어진 msg 송신
		
		// for(int i = 0; i < (2+opndCut*sizeof(int)); i++){
		// 	printf("[%d] = %d(%c)\n", i, data[i], data[i]);
		// }
		ret = sendTo, (hSocket, data, (2 + opndCut * sizeof(int)), 0, (SOCKADDR*)&servAdr, sizeof(servAdr));
		printf("Client> 서버에게 %d bytes 전송 \n", ret);
		if (ret == SOCKER_ERROR){
			ErrorHandling("ERROR> send() error");
		}
		// 5. 서버의 계산 결과(4byte) 수신 및 화면에 출력
		// rcvSum = 0;
		// rcvTarget = sizeof(result);
		// while(rcvSum < rcvTarget){
		// 	ret = recv(hSocket, (char*)&data[rcvSum], rcvTarget-rcvSum, 0);
		// 	rcvSum += ret;
		// 	if(ret <= 0){
		// 		flag = 0;
		// 		break;
		// 	}
		// 	result = *((int*)data);
			clntAdrSize = sizeof(clntAdr)
			recvfrom(hSocket, (char*)&result, sizeof(result), 0, (SOCKADDR*)&clntAdr, &clntAdrSize);

			printf("Client> 서버가 전달한 결과 = %d\n", result);
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