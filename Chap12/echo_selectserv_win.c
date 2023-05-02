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
	int				adrSz, strLen, fdNum, i, flag, addrlen;
	char			buf[BUF_SIZE];

	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 

	hServSock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_family		= AF_INET;
	servAdr.sin_addr.s_addr	= htonl(INADDR_ANY);
	servAdr.sin_port		= htons(9000);
	
	if(bind(hServSock, (SOCKADDR*) &servAdr, sizeof(servAdr))==SOCKET_ERROR)
		ErrorHandling("bind() error");
	if(listen(hServSock, 5)==SOCKET_ERROR)
		ErrorHandling("listen() error");

	TIMEVAL		timeout;
	fd_set		reads, cpyReads;

	FD_ZERO(&reads);			// read �׾Ƹ� �ʱ�ȭ
	FD_SET(hServSock, &reads);	// listen read �׾Ƹ��� �߰�

	while(1)
	{
		cpyReads = reads;	// ���纻 ����... 
		// reads: ���� event ��� ���ϵ� ����
		// select �� event�� �߻����� ���� ���� 0���� ǥ��.
		timeout.tv_sec	= 5;
		timeout.tv_usec = 5000;

		// select( , read, )
		fdNum = select(0, &cpyReads, 0, 0, &timeout);

		if (fdNum == SOCKET_ERROR) {
			printf("<ERROR> select error.\n");
			break;
		}

		if (fdNum == 0) {
			continue;  // timeout �̹Ƿ� select �ٽ� ȣ��.
		}

		// �߻� event ó�� loop
		for (i = 0; i < reads.fd_count; i++) {
			// event�� �߻��� ���Ͽ� ���� ó�� ����....
			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				// reads: event Ȯ�� ��� ��ü ������ ����
				// reads�� ���Ե� ���Ͽ� ���ؼ� ���� event��
				// �߻��ߴ� ���� Ȯ��.

				// 1. read event�� �߻��� ���.
				// 1.1 client�κ��� ���� ��û ����
				if (reads.fd_array[i] == hServSock) {

					printf("Server> client ���� ��� ��.\n");
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hServSock, (SOCKADDR*)&clntAdr, &adrSz);
					printf("connected client: Port:%d, IP:%s \n",
						clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

					FD_SET(hClntSock, &reads);
				}
				else {
					// 1.2 client�κ��� ������ ����.
					strLen = recv(reads.fd_array[i], buf, BUF_SIZE - 1, 0);
					if (strLen <= 0)    // close request!
					{
						closesocket(reads.fd_array[i]);
						printf("closed client: %d, StrLen:%d \n", reads.fd_array[i], strLen);
						FD_CLR(reads.fd_array[i], &reads);
					}
					else
					{
						// Ŭ���̾�Ʈ ���� ���						
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