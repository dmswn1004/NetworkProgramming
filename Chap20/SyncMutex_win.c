#include <stdio.h>
#include <windows.h>
#include <process.h>
#include <time.h>
#define NUM_THREAD	50
#define LOOP_CNT    100000
unsigned WINAPI threadInc(void * arg);
unsigned WINAPI threadDes(void * arg);

// ���� ������...
long long num = 0;

CRITICAL_SECTION cs;
HANDLE  hMutex;

int main(int argc, char *argv[]) 
{
	HANDLE tHandles[NUM_THREAD];
	int i;
	clock_t  start, end;

	InitializeCriticalSection(&cs);
	hMutex = CreateMutex( NULL, FALSE, NULL );  // Signaled

	start = clock();
	// ������ ����...
	for(i = 0; i < NUM_THREAD; i++) // 50�� ������ ����, 1/2->Inc, 1/2->Dec
	{
		if (i % 2 == 0) {	// ¦�� ��°
			tHandles[i] = (HANDLE)_beginthreadex( NULL, 0, threadInc, NULL, 0, NULL );
		}
		else {				// Ȧ�� ��°
			tHandles[i] = (HANDLE)_beginthreadex(NULL, 0, threadDes, NULL, 0, NULL);
		}
	}

	WaitForMultipleObjects( NUM_THREAD, tHandles, TRUE, INFINITE );
	end = clock();

	DeleteCriticalSection(&cs);
	CloseHandle(hMutex);

	printf("result: %lld \n", num);
	printf("> �ҿ� �ð� = %lf sec\n", (double)(end-start)/CLOCKS_PER_SEC);
	return 0;
}

unsigned WINAPI threadInc(void * arg) 
{
	int i;

	// num ����(����)...
	//EnterCriticalSection(&cs);
	WaitForSingleObject(hMutex, INFINITE); // hMutex : auto reset mode�� ����.
	for (i = 0; i < LOOP_CNT; i++ ) {
		num++;	
	}
	//LeaveCriticalSection(&cs);
	ReleaseMutex(hMutex);
	return 0;
}

unsigned WINAPI threadDes(void * arg)
{
	int i;

	// num ����(����)...
	// EnterCriticalSection(&cs);
	WaitForSingleObject(hMutex, INFINITE); // hMutex : auto reset mode�� ����.
	for (i = 0; i < LOOP_CNT; i++) {	
		num--;	
	}
	//LeaveCriticalSection(&cs);
	ReleaseMutex(hMutex);
	return 0;
}