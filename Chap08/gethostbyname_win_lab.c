#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
void ErrorHandling(char *message);

int main(int argc, char *argv[])
{
	WSADATA wsaData;
	int i;
	struct hostent *host;
	char dns[50];
	
	if(WSAStartup(MAKEWORD(2, 2), &wsaData)!=0)
		ErrorHandling("WSAStartup() error!"); 
	
	while(1){
		// 도메인 입력
		printf("> domain name : ");
		scanf("%s", dns);
		printf("> Entered domain name : %s\n", dns);

		// gethostbyname() 호출
		host = gethostbyname(dns);
		if(!host){
			printf("> gethostbyname() error\n");
			exit(1);
		}

		// 결과 출력
		// 1. 공식 도메인 이름
		printf("\n> official domain name : %s\n", host->h_name);
		// 2. 별병
		for(int i = 0; host->h_aliases[i]; i++){
			printf("- alias[%d] : %s\n", i, host->h_aliases[i]);
		}
		// 3. 주소 타입
		printf("- address type : %s\n", (host->h_addrtype == AF_INET)? "AF_INET" : "AF_INET6");
		// 4. IP주소
		for(int i = 0; host->h_addr_list[i]; i++){
			printf("- IP address[%d] : %s\n", i, inet_ntoa(*((struct in_addr*)host->h_addr_list[i])));
		}
	}

	WSACleanup();
	return 0;
}

void ErrorHandling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

