#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 30
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	SOCKET hSocket;
	FILE *fp;
	
	char buf[BUF_SIZE];
	int readCnt;
	SOCKADDR_IN servAdr;

	if(argc!=3) {
		printf("Usage: %s <IP> <port>\n", argv[0]);
		exit(1);
	}
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	fp = fopen("receive.dat", "wb");
	hSocket = socket(PF_INET, SOCK_STREAM, 0);   

	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= inet_addr(argv[1]);
	servAdr.sin_port		= htons(atoi(argv[2]));

	connect(hSocket, (SOCKADDR*)&servAdr, sizeof(servAdr));
	printf("Client> connect to server.\n");
	
	// 파일 크기 수신
	int fileSize;
	recv(hSocket, (char*)&fileSize, sizeof(fileSize), 0);
	printf("Client> File size : %d bytes\n", fileSize);

	// 파일 데이터 수신
	int receivedSize = 0;

	while(receivedSize < fileSize)
	{
		readCnt = recv(hSocket, buf, BUF_SIZE, 0);
		if( readCnt != 0 )
		{
			fwrite((void*)buf, 1, readCnt, fp);
			receivedSize += readCnt;
		}else {
			printf("Client> Connection closed unexpectedly.\n");
			break;
		}
		float percentage = (float)receivedSize / fileSize * 100;
		printf("Client> Receive %d bytes (total: %d bytes).\n", readCnt, fileSize);
		printf("수신률 : %.2f %%\n", percentage);
	}
	puts("Received file data");
	send(hSocket, "Thank you", 10, 0);
	fclose(fp);
	closesocket(hSocket);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}