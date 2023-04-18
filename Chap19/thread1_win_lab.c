#include <stdio.h>
#include <windows.h>
#include <process.h>    /* _beginthreadex, _endthreadex */
unsigned WINAPI ThreadFunc(void *arg);

int main(int argc, char *argv[]) 
{
	HANDLE		hThread;
	unsigned	threadID;
	int			param = 5;

	hThread =  (HANDLE)	_beginthreadex( 
							   // �Ķ���� �߰�
						NULL,			// ���� ����
						0,				// ���� ũ��
						ThreadFunc,		// �Լ� �̸�
						(void*)&param,	// �Լ� �Ķ� �ּ�
						0,				// ������ ���� �� ��� ����
						&threadID		
						);

	Sleep(3000);
	puts("> <main> end of main");
	return 0;
}

unsigned WINAPI ThreadFunc(void *arg)
{
	int i;
	int cnt;
	// ������ ���� �ڵ�.
	cnt = *((int*)arg);
	for (i = 0; i < cnt; i++) {
		printf("<thread> run %d.\n", i);
		Sleep(1000);
	}
	return 0;
}