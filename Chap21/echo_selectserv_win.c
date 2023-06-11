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

	// 1. WSAEventSelect() - sock - evtObj - events 매핑 - nonblocking
	newEvent = WSACreateEvent(); // manual reset event 객체 생성.
	ret = WSAEventSelect(hServSock, newEvent, FD_ACCEPT);
	if (ret == SOCKET_ERROR) {
		ErrorHandling("WSAEventSelect() error");
	}

	// 1.1 WSAEventSelect 호출 후 소켓과 이벤트 객체 저장 관리.
	hSockArr[numOfClntSock] = hServSock;
	hEventArr[numOfClntSock] = newEvent;
	numOfClntSock++;

	while(1)
	{
		// 2. 이벤트 생성 대기 및 이벤트 처리
		posInfo = WSAWaitForMultipleEvents(	numOfClntSock, hEventArr, FALSE, 
											WSA_INFINITE, FALSE);
		startIdx = posInfo - WSA_WAIT_EVENT_0;

		for (i= startIdx; i<numOfClntSock; i++)
		{
			// 각 이벤트에 대해서 signalled 상태 확인
			int sigEventIdx = WSAWaitForMultipleEvents(
								1, &hEventArr[i], TRUE, 0, FALSE);
			// non - signalled 인 경우 다음 event 확인을 위해 continue 실행...
			if (sigEventIdx == WSA_WAIT_FAILED || sigEventIdx == WSA_WAIT_TIMEOUT) {
				continue;
			}
			else {
				// 해당 이벤트 객체에 signal 발생한 경우...
				// ---> 각 이벤트 객체 무슨 이벤트 발생 했는 지 확인 및 동작 수행...
				// i 인덱스 위치에 이벤트 객체에 sig 발생...
				sigEventIdx = i;
				WSAEnumNetworkEvents(hSockArr[sigEventIdx], hEventArr[sigEventIdx], 
					&netEvents);

				// 2.1 연결 요청 이벤트 처리
				if (netEvents.lNetworkEvents & FD_ACCEPT) {
					// 현재 소켓 hSockArr[sigEventIdx] 에 client부터 연결 요청 수신된 상태

					// 서버 소켓은 con. req. 만 수신...-> accept 실행.
					adrSz = sizeof(clntAdr);
					hClntSock = accept(hSockArr[sigEventIdx], (SOCKADDR*)&clntAdr, &adrSz);
					printf("connected client: Port:%d, IP:%s \n",
						clntAdr.sin_port, inet_ntoa(clntAdr.sin_addr));

					// 새로운 소켓 hClntSock 생성...
					// 1. WSAEventSelect() - sock - evtObj - events 매핑 - nonblocking
					newEvent = WSACreateEvent(); // manual reset event 객체 생성.
					ret = WSAEventSelect(hClntSock, newEvent, FD_READ | FD_CLOSE);
					if (ret == SOCKET_ERROR) {
						ErrorHandling("WSAEventSelect() error");
					}
					// 1.1 WSAEventSelect 호출 후 소켓과 이벤트 객체 저장 관리.
					hSockArr[numOfClntSock] = hClntSock;
					hEventArr[numOfClntSock] = newEvent;
					numOfClntSock++;
				}

				// 2.2 데이터 수신 이벤트 처리
				if (netEvents.lNetworkEvents & FD_READ) {
					// 현재 소켓 hSockArr[sigEventIdx] 에 client부터 데이터가 수신된 상태

					strLen = recv(hSockArr[sigEventIdx], buf, BUF_SIZE - 1, 0);
										
					// 클라이언트 정보 얻기						
					addrlen = sizeof(clientaddr);
					getpeername(hSockArr[sigEventIdx], (SOCKADDR*)&clientaddr, &addrlen);

					buf[strLen] = '\0';
					printf("(Port:%d, IP:%s),Msg : %s \n",
						clientaddr.sin_port, inet_ntoa(clientaddr.sin_addr), buf);

					send(hSockArr[sigEventIdx], buf, strLen, 0);    // echo!
					
				}

				// 2.3 연결 종료 이벤트 처리
				if (netEvents.lNetworkEvents & FD_CLOSE) {
					// 현재 소켓 hSockArr[sigEventIdx] 에 client부터 데이터가 수신된 상태

					WSACloseEvent(hEventArr[sigEventIdx]);
					closesocket(hSockArr[sigEventIdx]);
					printf("closed client...\n");

					// 관리 대상에서 제외...					
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
