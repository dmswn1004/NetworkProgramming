#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
void ErrorHandling(char *message);
#define MAX_PACKET_SIZE  100

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	SOCKADDR_IN servAdr;
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	hSocket=socket(PF_INET, SOCK_STREAM, 0);   
	if(hSocket==INVALID_SOCKET)
		ErrorHandling("socket() error");
	
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family=AF_INET;
	servAdr.sin_addr.s_addr=inet_addr("127.0.0.1");
	servAdr.sin_port=htons(9000);

	// TCP 연결 요청...
	int ret;
	ret = connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr) );
	if (ret == SOCKET_ERROR) {
		printf("<ERROR> Client. connect() 실행 오류.\n");
		closesocket(hSocket);
		printf("Client> close socket...\n");
		WSACleanup();
		return 0;
	}
	else {
		printf("Client> connection established...\n");
	}
	
	// send/recv...
	int command, flag;
	char data[MAX_PACKET_SIZE];
	flag = 1;
	for (int i = 0; i < MAX_PACKET_SIZE; i++) {
		data[i] = i + 1;
	}
	int opndCnt, rcvSum, rcvTotal, result;
	char c;
	while (flag) 
	{
		printf("Client> 피연산자 수 입력: ");
		scanf("%d", &opndCnt);
		data[0] = opndCnt;

		for (int i = 0; i < opndCnt; i++) {
			printf("피연산자: ");
			scanf("%d", (int*)&data[1+i*sizeof(int)]);
		}
		rewind(stdin);
		//fgetc(stdin); // 마지막 enter 삭제.
		//c = getchar();
		printf("> 연산자를 입력하세요(+/-/*):");
		scanf("%c", &data[1+opndCnt*sizeof(int)]);

		ret = send(hSocket, data, 2 + opndCnt * sizeof(int), 0);
		printf("Client> sent %d bytes...\n", ret);

		rcvSum = 0;
		rcvTotal = sizeof(int);
		while (rcvSum < rcvTotal) {
			ret = recv(hSocket, (char*)&data[rcvSum], rcvTotal - rcvSum, 0);
			if (ret <= 0) {
				flag = 0;
				break;
			}
			else {
				rcvSum += ret;
				printf("Client> recv %d bytes(rcvSum = %d bytes).\n", ret, rcvSum);
			}
		}
		result = *((int *)data);
		printf("Client> 연산 결과 = %d\n", result);
	}
	closesocket(hSocket);
	printf("Client> close socket...\n");
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}