// Snow 2017-04-10
#pragma once

#include <Windows.h>

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT InitNetwork();
BOOL SendData(INT Sock, ULONG Address, LPWSTR szData, ULONG iDataSize);
VOID CleanNetwork(INT Sock);