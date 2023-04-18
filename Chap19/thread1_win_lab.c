#include <stdio.h>
#include <windows.h>
#include <process.h>    /* _beginthreadex, _endthreadex */
unsigned WINAPI ThreadFunc(void *arg);

int main(int argc, char *argv[]) 
{
	HANDLE		hThread;
	unsigned	threadID;
	int		param = 3;

	hThread =  (HANDLE)	_beginthreadex( 
							   // 파라메터 추가
						NULL,			// 보안 관련
						0,			// 스택 크기
						ThreadFunc,		// 함수 이름
						(void*)&param,		// 함수 파람 주소
						0,			// 스레드 생성 후 즉시 실행
						&threadID		
						);

	// Sleep(3000);
	DWORD ret;
	ret = WaitForSingleObject(hThread, INFINITE);
	if (ret == WAIT_FAILED) {
		printf("<EROR> wait failed.\n");
		return (-1);
	}else if(ret == WAIT_TIMEOUT) {
		printf("<ERROR> timeout error.\n");
		return -1;
	}else if(ret == WAIT_OBJECT_0){
		printf("<SIGNALLED>.\n");
	}
	
	puts("> <main> end of main");
	return 0;
}

unsigned WINAPI ThreadFunc(void *arg)
{
	int i;
	int cnt;
	// 스레드 동작 코딩.
	cnt = *((int*)arg);
	for (i = 0; i < cnt; i++) {
		printf("<thread> run %d.\n", i);
		Sleep(1000);
	}
	return 0;
}
