// Snow 2017-04-10
#pragma once

#include <Windows.h>

typedef struct {
	HWND hMainWnd;
	INT Sock;
} THREAD_DATA;

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT InitNetwork();
DWORD WINAPI NetThreadProc(LPVOID lParam);
VOID CleanNetwork(INT Sock);