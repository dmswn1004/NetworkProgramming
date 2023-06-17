#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 100
void ErrorHandling(char *message);

int main(void)
{
	WSADATA			wsaData;
	SOCKET			hServSock, hClntSock;
	SOCKADDR_IN		servAdr, clntAdr, clientaddr;
	int				adrSz, strLen, fdNum, i, flag, addrlen;
	char			buf[BUF_SIZE];

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= inet_addr("127.0.0.1");
	servAdr.sin_port		= htons(9000);
	
	if(bind(hServSock, (SOCKADDR*) &servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 5)==SOCKET_ERROR)
		ErrorHandling("listen() error");

	fd_set cpyReads, reads;
	TIMEVAL timeout;

	FD_ZERO(&reads);
	FD_SET(hServSock,&reads);

	while(1)
	{
		cpyReads = reads;
		timeout.tv_sec = 5;
		timeout.tv_usec = 5000;
		// 1. select( , cpyReads, ) // 여러개 소켓 대상 event 확인
		fdNum = select(0, &cpyReads, 0, 0, &timeout);
		if (fdNum == SOCKET_ERROR) {
			printf("<ERROR> select socket error.\n");
		}
		else if (fdNum == 0) {
			continue;
		}

		// 2. 여러 소켓 대상 확인된 event 처리
		// for(등록된 모든 소켓에 대해서 loop)
		//  --- event 확인... 모든 소켓 대상 FD_ISSET(i, cpyReads)
		//     -- read Event 경우, 처리(data recv, cnt req 처리)
		for (i = 0; i < reads.fd_count; i++) {

			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				// 해당 소켓에 read event 발생....
				if (reads.fd_array[i] == hServSock) {
					// 서버 소켓은 con. req. 만 수신...-> accept 실행.
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
					printf("connected new client...\n");

					FD_SET(hClntSock, &reads);
				}
				else {
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strLen <= 0)    // close request!
					{
						closesocket(reads.fd_array[i]);
						
						FD_CLR(reads.fd_array[i], &reads);
					}
					else
					{
						// 클라이언트 정보 얻기						
						addrlen = sizeof(clientaddr);
						getpeername(reads.fd_array[i], (SOCKADDR*)&clientaddr, &addrlen);

						buf[strLen] = '\0';
						
						int opndCnt = buf[0];
						int* data = (int*)&buf[1];
						char op = buf[1 + opndCnt * sizeof(int)];
						
						printf("Server> recv %d bytes(rcvSum = %d bytes).\n", 1 + opndCnt * sizeof(int), 1 + opndCnt * sizeof(int));
						int result = calculate(opndCnt, data, op);
						printf("Server> 계산 결과(%d) client로 전송.\n", result);
						send(reads.fd_array[i], (char*)&result, sizeof(result), 0); // echo!
					}
				}
			}
		}
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
			result += data[i];
		}
		break;
	case '-':
		for (int i = 1; i < opndCnt; i++) {
			result -= data[i];
		}
		break;
	case '*':
		for (int i = 1; i < opndCnt; i++) {
			result *= data[i];
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
