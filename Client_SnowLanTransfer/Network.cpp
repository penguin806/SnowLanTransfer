// Snow 2017-04-10
#include <WinSock2.h>
#include "Header.h"
#pragma comment(lib,"ws2_32.lib")

#define BUFFER_LEN 512

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
		WSACleanup();
		return INVALID_SOCKET;
	}

	int error = WSAGetLastError();

	SOCKADDR_IN BindAddress;
	ZeroMemory(&BindAddress, sizeof(BindAddress));
	BindAddress.sin_family = AF_INET;
	BindAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	BindAddress.sin_port = htons(2017);

	iResult = bind(Sock, (const sockaddr*)&BindAddress, sizeof(BindAddress));
	if (iResult != 0)
	{
		closesocket(Sock);
		WSACleanup();
		return INVALID_SOCKET;
	}

	int error2 = WSAGetLastError();

	return Sock;
}

DWORD WINAPI NetThreadProc(LPVOID lParam)
{
	if (lParam == NULL)
	{
		MessageBox(NULL, TEXT("Thread Error"), TEXT("Error"), MB_OK | MB_ICONERROR);
		return 0;
	}
	THREAD_DATA dataPassed = *((THREAD_DATA *)lParam);
	TCHAR szBuffer[BUFFER_LEN + 1];
	INT iResult, iLength = sizeof(SOCKADDR_IN);
	SOCKADDR_IN IncomingAddr;

	ZeroMemory(&IncomingAddr, sizeof(IncomingAddr));

	while (TRUE)
	{
		ZeroMemory(szBuffer, sizeof(szBuffer));
		iResult = recvfrom(dataPassed.Sock, (CHAR*)szBuffer, sizeof(szBuffer) - 2, 0,
			(sockaddr*)&IncomingAddr, &iLength);

		//int error = WSAGetLastError();
		ParseData(dataPassed.hOutput, szBuffer, iResult, IncomingAddr.sin_addr);
	}

	return 0;
}

VOID ParseData(HWND hOutput, TCHAR szDataRecv[], UINT RecvSize, IN_ADDR fromAddress)
{
	TCHAR *szDisplayBuffer = (TCHAR *)LocalAlloc(LMEM_ZEROINIT, BUFFER_LEN * 60);
	TCHAR *szDisplayBufferB = (TCHAR *)LocalAlloc(LMEM_ZEROINIT, BUFFER_LEN * 60);
	GetWindowText(hOutput, szDisplayBuffer, BUFFER_LEN * 30);

	if (RecvSize != BUFFER_LEN * 2)
	{
		wsprintf(szDisplayBufferB, TEXT("%s[%s]\r\nData Error\r\n"), szDisplayBuffer,
			inet_ntoa(fromAddress));
		SetWindowText(hOutput, szDisplayBufferB);
		return;
	}
	
	TCHAR szType[6] = { 0 }, szBuffer[BUFFER_LEN] = { 0 }, *p = szDataRecv;
	for (int i = 0; i < 5; i++)
		szType[i] = *p++;

	for (int i = 0; *p && p < szDataRecv + BUFFER_LEN; i++, p++)
	{
		szBuffer[i] = *p;
	}

	if (lstrcmp(szType, TEXT("#Sen#")) == 0)
	{
		LPWSTR UnicodeIpAddr = ANSIToUnicode(inet_ntoa(fromAddress));
		wsprintf(szDisplayBufferB, TEXT("%s[%s] Message:\r\n%s\r\n"), szDisplayBuffer,
			UnicodeIpAddr, szBuffer);
		free(UnicodeIpAddr);

		SetWindowText(hOutput, szDisplayBufferB);
	}
	else if (lstrcmp(szType, TEXT("#Dow#")) == 0)
	{

	}
	else if (lstrcmp(szType, TEXT("#Exe#")) == 0)
	{
		
	}
	else
	{
		wsprintf(szDisplayBufferB, TEXT("%s[%s]\r\nUnknown Data\r\n"), szDisplayBuffer,
			inet_ntoa(fromAddress));
		SetWindowText(hOutput, szDisplayBufferB);
	}
	LocalFree(szDisplayBuffer);
	LocalFree(szDisplayBufferB);
}

VOID CleanNetwork(INT Sock)
{
	if (Sock != INVALID_SOCKET || Sock != 0)
	{
		closesocket(Sock);
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
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)result, textlen);
	return result;
}