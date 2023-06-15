#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 100
int calculate(int opndCnt, int data[], char* op);
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
				if (reads.fd_array[i] == hServSock) {
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
					printf("connected client: Port:%d, IP:%s \n",
						clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

					FD_SET(hClntSock, &reads);
				}
				else {
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strLen <= 0)    // close request!
					{
						closesocket(reads.fd_array[i]);
						printf("closed client: %d, StrLen:%d \n", hClntSock, strLen);
						
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

						int ret, rcvSum, rcvTotal, result;
						char opndCnt;
						flag = 1;
						
						while (flag)
						{
							printf("Server> 계산 요청 대기.\n");

							// 1. 패킷 size 가 담긴 1 바이트 수신...
							ret = recv(hClntSock, &opndCnt, 1, 0);

							rcvSum = 0;
							rcvTotal = (int)opndCnt * sizeof(int) + 1;
							while (rcvSum < rcvTotal) {
								ret = recv(hClntSock, (char*)&buf[rcvSum], rcvTotal - rcvSum, 0);
								if (ret <= 0) {
									flag = 0;
									break;
								}
								else {
									rcvSum += ret;
									printf("Server> recv %d bytes(rcvSum = %d bytes).\n", ret, rcvSum);
								}
							}

							if (flag == 1) {
								result = calculate((int)opndCnt, (int*)buf, buf[rcvTotal - 1]);
								printf("Server> 계산 결과(%d) client로 전송.\n", result);
								send(hClntSock, (char*)&result, sizeof(result), 0);
							}

						send(hClntSock, buf, strLen, 0);    // echo!
					}
				}
			}
		}
	}
	closesocket(hServSock);
	WSACleanup();
	return 0;
}

int calculate(int opndCnt, int data[], char* op)
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