#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

void ErrorHandling(char *message);
void ShowSocketBufSize(SOCKET sock);

int main(int argc, char *argv[])
{
	WSADATA  wsaData;
	SOCKET tcp_sock, udp_sock;
	int state;
	int sock_type;
	int optlen;
	 
	if(WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		ErrorHandling("WSAStartup() error!");
	
	optlen = sizeof(sock_type);
	tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
	udp_sock = socket(PF_INET, SOCK_DGRAM, 0);	
	printf("SOCK_STREAM: %d \n", SOCK_STREAM);  // 1
	printf("SOCK_DGRAM: %d \n", SOCK_DGRAM);    // 2

	// tcp 소켓의 소켓 타입 확인 
	state = getsockopt(tcp_sock, SOL_SOCKET, SO_TYPE, (char*)&sock_type, &optlen);
	if(state == SOCKET_ERROR){
		ErrorHandling("1. getsockopt() error");
	}
	printf("> socket type(TCP) : %d\n", sock_type);
	// udp 소켓의 소켓 타입 확인 
	state = getsockopt(udp_sock, SOL_SOCKET, SO_TYPE, (char*)&sock_type, &optlen);
	if(state == SOCKET_ERROR){
		ErrorHandling("2. getsockopt() error");
	}
	printf("> socket type(UDP) : %d\n", sock_type);

	WSACleanup();
	return 0;
}


void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}
