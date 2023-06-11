#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUF_SIZE 100
void ErrorHandling(char *message);
void CompressSockets(SOCKET hSockArr[], int idx, int total);
void CompressEvents(SOCKET hEventArr[], int idx, int total);

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

#if 0
	fd_set cpyReads, reads;
	TIMEVAL timeout;

	FD_ZERO(&reads);
	FD_SET(hServSock,&reads);
#endif

	int			ret, numOfClntSock=0, posInfo, startIdx;
	WSAEVENT	newEvent, hEventArr[WSA_MAXIMUM_WAIT_EVENTS];
	SOCKET		hSockArr[WSA_MAXIMUM_WAIT_EVENTS];
	WSANETWORKEVENTS  netEvents;

	// 1. WSAEventSelect() - sock - evtObj - events ���� - nonblocking
	newEvent = WSACreateEvent(); // manual reset event ��ü ����.
	ret = WSAEventSelect(hServSock, newEvent, FD_ACCEPT);
	if (ret == SOCKET_ERROR) {
		ErrorHandling("WSAEventSelect() error");
	}

	// 1.1 WSAEventSelect ȣ�� �� ���ϰ� �̺�Ʈ ��ü ���� ����.
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while(1)
	{
		// 2. �̺�Ʈ ���� ��� �� �̺�Ʈ ó��
		posInfo = WSAWaitForMultipleEvents(	numOfClntSock, hEventArr, FALSE, 
											WSA_INFINITE, FALSE);
		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (i= startIdx; i<numOfClntSock; i++)
		{
			// �� �̺�Ʈ�� ���ؼ� signalled ���� Ȯ��
			int sigEventIdx = WSAWaitForMultipleEvents(
								1, &hEventArr[i], TRUE, 0, FALSE);
			// non - signalled �� ��� ���� event Ȯ���� ���� continue ����...
			if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT) {
				continue;
			}
			else {
				// �ش� �̺�Ʈ ��ü�� signal �߻��� ���...
				// ---> �� �̺�Ʈ ��ü ���� �̺�Ʈ �߻� �ߴ� �� Ȯ�� �� ���� ����...
				// i �ε��� ��ġ�� �̺�Ʈ ��ü�� sig �߻�...
				sigEventIdx = i;
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], 
					&netEvents);

				// 2.1 ���� ��û �̺�Ʈ ó��
				if (netEvents.lNetworkEvents & FD_ACCEPT) {
					// ���� ���� hSockArr[sigEventIdx] �� client���� ���� ��û ���ŵ� ����

					// ���� ������ con. req. �� ����...-> accept ����.
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &adrSz);
					printf("connected client: Port:%d, IP:%s \n",
						clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

					// ���ο� ���� hClntSock ����...
					// 1. WSAEventSelect() - sock - evtObj - events ���� - nonblocking
					newEvent = WSACreateEvent(); // manual reset event ��ü ����.
					ret = WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);
					if (ret == SOCKET_ERROR) {
						ErrorHandling("WSAEventSelect() error");
					}
					// 1.1 WSAEventSelect ȣ�� �� ���ϰ� �̺�Ʈ ��ü ���� ����.
					hSockArr[numOfClntSock] = hClntSock;
					hEventArr[numOfClntSock] = newEvent;
					numOfClntSock++;
				}

				// 2.2 ������ ���� �̺�Ʈ ó��
				if (netEvents.lNetworkEvents & FD_READ) {
					// ���� ���� hSockArr[sigEventIdx] �� client���� �����Ͱ� ���ŵ� ����

					strLen = recv(hSockArr[sigEventIdx], buf, BUF_SIZE - 1, 0);
										
					// Ŭ���̾�Ʈ ���� ���						
					addrlen = sizeof(clientaddr);
					getpeername(hSockArr[sigEventIdx], (SOCKADDR*)&clientaddr, &addrlen);

					buf[strLen] = '\0';
					printf("(Port:%d, IP:%s),Msg : %s \n",
						clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr), buf);

					send(hSockArr[sigEventIdx], buf, strLen, 0);    // echo!
					
				}

				// 2.3 ���� ���� �̺�Ʈ ó��
				if (netEvents.lNetworkEvents & FD_CLOSE) {
					// ���� ���� hSockArr[sigEventIdx] �� client���� �����Ͱ� ���ŵ� ����

					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					printf("closed client...\n");

					// ���� ��󿡼� ����...					
					numOfClntSock--;
					CompressSockets(hSockArr, sigEventIdx, numOfClntSock);
					CompressEvents(hEventArr, sigEventIdx, numOfClntSock);
				}
			}
		}

#if 0
		cpyReads = reads;
		timeout.tv_sec = 5; // 5sec
		timeout.tv_usec = 5000; // 5msec
		// 1. select( , cpyReads, ) // ������ ���� ��� event Ȯ��
		fdNum = select(0, &cpyReads, 0, 0, &timeout);
		if (fdNum == SOCKET_ERROR) {
			printf("<ERROR> select socket error.\n");
		}
		else if (fdNum == 0) {
			continue;
		}

		// 2. ���� ���� ��� Ȯ�ε� event ó��
		// for(��ϵ� ��� ���Ͽ� ���ؼ� loop)
		//  --- event Ȯ��... ��� ���� ��� FD_ISSET(i, cpyReads)
		//     -- read Event ���, ó��(data recv, cnt req ó��)
		for (i = 0; i < reads.fd_count; i++) {

			if (FD_ISSET(reads.fd_array[i], &cpyReads)) {
				// �ش� ���Ͽ� read event �߻�....
				if (reads.fd_array[i] == hServSock) {
					// ���� ������ con. req. �� ����...-> accept ����.
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
#endif

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

void CompressSockets(SOCKET hSockArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; i++) {
		hSockArr[i] = hSockArr[i + 1];
	}
}

void CompressEvents(SOCKET hEventArr[], int idx, int total)
{
	int i;
	for (i = idx; i < total; i++) {
		hEventArr[i] = hEventArr[i + 1];
	}
}
