// Snow 2017-04-10
#pragma once

#include <Windows.h>

#define BUFFER_LEN 512
#define OPTION_NUM 3

struct snow_data
{
	INT Sock;
	INT recvSock;
	HWND hOutput;
	HANDLE hFile;
};
typedef struct snow_data SNOWDATA;

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT InitNetwork();
BOOL SendData(INT Sock, ULONG Address, LPTSTR szData, ULONG iDataSize);
VOID CleanNetwork(SNOWDATA Data);
INT InitListenClient();
DWORD WINAPI ClientMessageProc(LPVOID lParam);
HANDLE CreateLogFile();
VOID FormatRecvMessage(HWND hOutput, TCHAR *recvBuffer, INT recvLength, IN_ADDR fromAddr, HANDLE hLogFile);
VOID WriteLogToFile(HANDLE hFile, const TCHAR *Log);
wchar_t * ANSIToUnicode(const char* str);
char * UnicodeToANSI(const wchar_t *str);