#include <stdio.h>
#include <windows.h>
#include <process.h> 
#define STR_LEN		100

unsigned WINAPI NumberOfA(void *arg);
unsigned WINAPI NumberOfOthers(void *arg);

static char str[STR_LEN];
static HANDLE hEvent;

int main(int argc, char *argv[]) 
{	
	HANDLE  hThread1, hThread2;

	// 1. 이벤트 객체 생성하기 (manual reset, 초기값 FALSE)
	hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	
	hThread1=(HANDLE)_beginthreadex(NULL, 0, NumberOfA, NULL, 0, NULL);
	hThread2=(HANDLE)_beginthreadex(NULL, 0, NumberOfOthers, NULL, 0, NULL);

	// 2. 사용자가 스트링 입력하기
	printf("Input string : ", stdout);
	fgets(str, STR_LEN, stdin);
	SetEvent(hEvent); // signal 부여 hEvent에 대해서...
	
	// 3. 이벤트 객체 대상으로 signaled 상태로 설정하여 스레드 진행
	WaitForSingleObject(hThread1, INFINITE);
	WaitForSingleObject(hThread2, INFINITE);
	ResetEvent(hEvent);
 	CloseHandle(hEvent);
    return 0;
}

// str string 배열에 대해서 'A' 갯수 counting...
unsigned WINAPI NumberOfA(void *arg) 
{
	int i, cnt=0;
	
	// 이벤트 획득. 'A'글자 수 계수.
	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] == 'A') {
			cnt++;
		}
	}
	printf("> Number of 'A' : %d\n", cnt);
	return 0;
}

unsigned WINAPI NumberOfOthers(void *arg) 
{
	int i, cnt=0;
	
	// 이벤트 획득. 'A'외의 글자 수 계수.
	WaitForSingleObject(hEvent, INFINITE);
	for (i = 0; str[i] != 0; i++) {
		if (str[i] != 'A' && str[i] != '\n') {
			cnt++;
		}
	}
	printf("> Number of others : %d\n", cnt);
	return 0;
}