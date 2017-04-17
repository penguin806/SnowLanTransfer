// Snow 2017-04-10
#include <WinSock2.h>
#include "Header.h"

#pragma comment(lib, "ws2_32.lib")

INT InitNetwork()
{
	WORD wsVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	INT iResult;
	INT Sock;

	iResult = WSAStartup(wsVersion, &wsaData);
	if (iResult != 0)
	{
		return INVALID_SOCKET;
	}
	
	Sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (Sock == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	const BOOL so_Broadcast = TRUE;
	iResult = setsockopt(Sock, SOL_SOCKET, SO_BROADCAST, (const char*)&so_Broadcast, sizeof(so_Broadcast));
	if (iResult != 0)
	{
		closesocket(Sock);
		WSACleanup();
		return INVALID_SOCKET;
	}

	return Sock;
}

INT InitListenClient()
{
	INT recvSock = 0;
	
	recvSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (recvSock == INVALID_SOCKET)
	{
		return INVALID_SOCKET;
	}

	const int reuse = 1;
	setsockopt(recvSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse));

	SOCKADDR_IN Addr;
	ZeroMemory(&Addr, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_addr.S_un.S_addr = INADDR_ANY;
	Addr.sin_port = htons(2018);

	INT iResult = bind(recvSock, (const sockaddr *)&Addr, sizeof(Addr));
	if (iResult != 0)
	{
		closesocket(recvSock);
		return INVALID_SOCKET;
	}

	return recvSock;
}

DWORD WINAPI ClientMessageProc(LPVOID lParam)
{
	SNOWDATA Data;
	CopyMemory(&Data, (const void *)lParam, sizeof(SNOWDATA));

	if (Data.recvSock == INVALID_SOCKET || Data.recvSock == 0)
	{
		return 0;
	}

	SOCKADDR_IN IncomingAddr;
	ZeroMemory(&IncomingAddr, sizeof(SOCKADDR_IN));
	INT iLength = sizeof(IncomingAddr);
	TCHAR *recvBuffer = (TCHAR *)LocalAlloc(LMEM_ZEROINIT, BUFFER_LEN * sizeof(TCHAR));

	INT iResult;
	while (TRUE)
	{
		ZeroMemory(recvBuffer, BUFFER_LEN * sizeof(TCHAR));
		iResult = recvfrom(Data.recvSock, (char *)recvBuffer, BUFFER_LEN * sizeof(TCHAR),
			0, (sockaddr *)&IncomingAddr, &iLength);
		FormatRecvMessage(Data.hOutput, recvBuffer, sizeof(TCHAR) * BUFFER_LEN,
			IncomingAddr.sin_addr, Data.hFile);

		if (iResult == SOCKET_ERROR)
		{
			break;
		}
	}

	LocalFree(recvBuffer);
	ExitThread(0);
	return TRUE;
}

VOID FormatRecvMessage(HWND hOutput, TCHAR *recvBuffer, INT recvLength, IN_ADDR fromAddr, HANDLE hLogFile)
{
	TCHAR *Buffer = (TCHAR *)LocalAlloc(LMEM_ZEROINIT, BUFFER_LEN * sizeof(TCHAR));
	TCHAR *szDisplayBuffer = (TCHAR *)LocalAlloc(LMEM_ZEROINIT, BUFFER_LEN * sizeof(TCHAR) * 100);

	LPTSTR UnicodeIpAddr = ANSIToUnicode(inet_ntoa(fromAddr));
	wsprintf(Buffer, TEXT("RECV FROM %s:\r\n%s\r\n"), UnicodeIpAddr, recvBuffer);
	free(UnicodeIpAddr);

	WriteLogToFile(hLogFile, Buffer);

	GetWindowText(hOutput, szDisplayBuffer, BUFFER_LEN * sizeof(TCHAR) * 100);
	lstrcat(szDisplayBuffer, Buffer);
	SetWindowText(hOutput, szDisplayBuffer);

	LocalFree(Buffer);
	LocalFree(szDisplayBuffer);
}

HANDLE CreateLogFile()
{
	HANDLE hFile = NULL;

	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH - 1);

	TCHAR *p = szPath;
	for (; p < szPath + MAX_PATH && *p; p++);
	for (; p > szPath && *p != TEXT('\\'); p--)
	{
		*p = 0;
	}

	lstrcat(szPath, TEXT("Server_SnowLanTransfer.log"));

	hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);


	if (hFile != INVALID_HANDLE_VALUE && hFile != NULL)
	{
		SetFilePointer(hFile, 0, NULL, FILE_END);
	}

	return hFile;
}


VOID WriteLogToFile(HANDLE hFile, const TCHAR *Log)
{
	TCHAR szBuffer[BUFFER_LEN] = { 0 };
	SYSTEMTIME Time;
	ZeroMemory(&Time, sizeof(Time));
	GetLocalTime(&Time);

	wsprintf(szBuffer, TEXT("[%04d-%02d-%02d %02d:%02d:%02d] %s"), Time.wYear,
		Time.wMonth, Time.wDay, Time.wHour, Time.wMinute, Time.wSecond);

	DWORD ActualWritten = 0;
	if (hFile != INVALID_HANDLE_VALUE && hFile != 0)
	{
		WriteFile(hFile, (LPCVOID)szBuffer, lstrlen(szBuffer) * sizeof(TCHAR),
			&ActualWritten, NULL);
	}
}

BOOL SendData(INT Sock, ULONG Address, LPTSTR szData, ULONG iDataSize)
{
	if (Sock == 0 || Sock == INVALID_SOCKET)
	{
		return FALSE;
	}

	INT iResult;

	SOCKADDR_IN SockInfo;
	ZeroMemory(&SockInfo, sizeof(SockInfo));
	SockInfo.sin_family = AF_INET;
	SockInfo.sin_port = htons(2017);
	SockInfo.sin_addr.S_un.S_addr = Address;

	iResult = sendto(Sock, (const char *)szData, iDataSize, 
		0, (const sockaddr*)&SockInfo, sizeof(SockInfo));
	if (iResult < (INT)iDataSize)
	{
		return FALSE;
	}

	return TRUE;
}

VOID CleanNetwork(SNOWDATA Data)
{
	if (Data.Sock != INVALID_SOCKET || Data.Sock != 0)
	{
		shutdown(Data.Sock, SD_BOTH);
		closesocket(Data.Sock);
	}

	if (Data.recvSock != INVALID_SOCKET || Data.recvSock != 0)
	{
		shutdown(Data.recvSock, SD_BOTH);
		closesocket(Data.recvSock);
	}

	WSACleanup();
}

// Copy from http://blog.csdn.net/linuxandroidwince/article/details/7527232 2017-04-11
wchar_t * ANSIToUnicode(const char* str)
{
	int textlen;
	wchar_t * result;
	textlen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	result = (wchar_t *)malloc((textlen + 1) * sizeof(wchar_t));
	memset(result, 0, (textlen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPTSTR)result, textlen);
	return result;
}