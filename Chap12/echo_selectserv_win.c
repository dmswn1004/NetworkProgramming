#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 1024
void ErrorHandling(char *message);

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hServSock, hClntSock;
	SOCKADDR_IN		servAdr, clntAdr, clientaddr;
	int			adrSz, strLen, fdNum, i, flag, addrlen;
	char			buf[BUF_SIZE];

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr		= htonl(INADDR_ANY);
	servAdr.sin_port		= htons(9000);
	
	if(bind(hServSock, (SOCKADDR*) &servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 5)==SOCKET_ERROR)
		ErrorHandling("listen() error");

	TIMEVAL		timeout;
	fd_set		reads, cpyReads;

	FD_ZERO(&reads);		// read 항아리 초기화
	FD_SET(hServSock, &reads);	// listen read 항아리에 추가

	while(1)
	{
		cpyReads = reads;	// 복사본 생성... 
		// reads: 현재 event 대기 소켓들 정보
		// select 후 event가 발생하지 않은 소켓 0으로 표시.
		timeout.tv_sec	= 5;
		timeout.tv_usec = 5000;

		// select( , read, )
		fdNum = select(0, &cpyReads, 0, 0, &timeout);

		if (fdNum == SOCKET_ERROR) {
			printf("<ERROR> select error.\n");
			break;
		}

		if (fdNum == 0) {
			continue;  // timeout 이므로 select 다시 호출.
		}

		// 발생 event 처리 loop
		for (i = 0; i < reads.fd_count; i++) {
			// event가 발생한 소켓에 대한 처리 진행....
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				// reads: event 확인 대상 전체 소켓을 포함
				// reads에 포함된 소켓에 대해서 현재 event가
				// 발생했는 지를 확인.

				// 1. read event가 발생한 경우.
				// 1.1 client로부터 연결 요청 수신
				if (reads.fd_array[i] == hServSock) {

					printf("Server> client 연결 대기 중.\n");
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
					printf("connected client: Port:%d, IP:%s \n",
						clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

					FD_SET(hClntSock, &reads);
				}
				else {
					// 1.2 client로부터 데이터 수신.
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strLen <= 0)    // close request!
					{
						closesocket(reads.fd_array[i]);
						printf("closed client: %d, StrLen:%d \n", reads.fd_array[i], strLen);
						FD_CLR(reads.fd_array[i], &reads);
					}
					else
					{
						// 클라이언트 정보 얻기						
						addrlen = sizeof(clientaddr);
						getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);

						buf[strLen] = '\0';
						printf("(Port:%d, IP:%s),Msg : %s \n",
							clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr), buf);
						send(reads.fd_array[i], buf, strLen, 0);    // echo!
					}
				}
			}
		}
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
