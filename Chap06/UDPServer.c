#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
#define MAX_PACKET_SIZE 120
int calculate(int opndCut, int data[], char op);
void ErrorHandling(char *message);

int main()
{
	WSADATA		wsaData;
	SOCKET		hServSock, hClntSock;
	int			i = 0;
	SOCKADDR_IN servAdr, clntAdr;
	int			clntAdrSize, ret, flag;
	char opndCut;
	char data[MAX_PACKET_SIZE];
	int rcvTarget, rcvSum = 0, result;

	if( WSAStartup(MAKEWORD(2, 2), &wsaData) !=0 )
		ErrorHandling("WSAStartup() error!"); 
	
	// 1. 소켓 생성 (연결 수신 소켓 : 리스닝 소켓, 서버 소켓)
	hServSock=socket(PF_INET, SOCK_DGRAM, 0);   // UDP 소켓 생성
	if(hServSock==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	// 주소 설정
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family			=AF_INET;
	servAdr.sin_addr.s_addr		= inet_addr("127.0.0.1"); // 32bits 숫자, 네트워크 바이트 순서, Big endian으로 변경해주는 함수 사용
								// ntohl(inet_addr("127.0.0.1")); 네트워크에서 호스트로 (big endian->little endian)으로 변경
								// htonl( INADDR_ANY ); 
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

	// 4. accept() : 클라이언트의 연결 수락, (중요) 전송용 새로운 소켓 생성
	// hClntSock : 데이터 전송 소켓, 연결된 client와 1:1 연결
	while(1){
		// clntAdrSize = sizeof(clntAdr);
		// printf("Server> vaiting client...\n");
		// hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &clntAdrSize);
		// if(hClintSock == SOCKET_ERROR) {
		// 	ErrorHandling("accept() error");
		// }
		// printf("Server> client(IP:%s, Ports:%d) is connected\n", inet_ntoa(clintAdr.sin_addr), ntohs(clintAdr.sin_port));

		
		// flag = 1;
		// 서버 App 프로토콜 구현
		// while (flag){
			printf("Server> 클라이언트의 계산 요청 대기\n");
			// 0. 수신시 상새 client의 종료 확인 및 loop에서 탈출

			// 1. 1byte 읽어서 피연산자 수 및 추가로 읽어야 할 데이터량 계산
			ret = recvfrom(hServSock, data, MAX_PACKET_SIZE, 0, (SOCKADDR*)&clintAdr, &clntAdrSize);
			if (ret <= 0){
				printf("Server> recv() error\n");
				break;
			}
			// 2. 나머지 데이터 수신...loop
			// rcvSum = 0;
			// rcvTarget = opndCut * sizeof(int) + 1;
			// while (rcvSum < rcvTarget){
			// 	ret = recv(hClintSock, &data[rcvSum], rcvTarget - rcvSum, 0);
			// 	if(ret <= 0){
			// 		flag = 0;
			// 		break;
			// 	}
			// 	rcvSum += ret;
			// }
			
			//if(flag == 1){
			// 3. 계산
			result = calculate((int)data[0], (int*)&data[1], data[ret-1]);
			printf("Server> 게산 결과 = %d\n", result);
			// 4. 결과 클라이언트로 전송
			ret = sendTo(hServSock, (char*)&result, sizeof(result), 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
			if (ret == SOCKET_ERROR){
				printf("Server> send() error\n");
				break;
			}
			//}

			
		// }
		
		// closesocket(hClntSock); // 클라이언트 소켓: 클라이언트의 데이터 송수신
	}

	closesocket(hServSock); // 서버 소켓: 연결 청취(listening) 소켓
	WSACleanup();
	return 0;
}

int calculate(int opndCut, int data[], char op){
	int result = data[0];
	switch (op){
	case '+':
		for(int i = 1; i < opndCut; i++){
			result += data[i];
		}
		break;
	case '-':
		for(int i = 1; i < opndCut; i++){
			result -= data[i];
		}
		break;
	case '*':
		for(int i = 1; i < opndCut; i++){
			result *= data[i];
		}
		break;
	default:
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