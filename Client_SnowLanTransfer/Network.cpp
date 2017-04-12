// Snow 2017-04-10
#include <WinSock2.h>
#include <UrlMon.h>
#include "Header.h"
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "urlmon.lib")

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

	//int error = WSAGetLastError();

	SOCKADDR_IN BindAddress;
	ZeroMemory(&BindAddress, sizeof(BindAddress));
	BindAddress.sin_family = AF_INET;
	BindAddress.sin_addr.S_un.S_addr = INADDR_ANY;
	BindAddress.sin_port = htons(2017);

	iResult = bind(Sock, (const sockaddr*)&BindAddress, sizeof(BindAddress));
	if (iResult != 0)
	{
		//int error2 = WSAGetLastError();
		closesocket(Sock);
		WSACleanup();
		return INVALID_SOCKET;
	}

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
		wsprintf(szDisplayBufferB, TEXT("%s[%s]\r\nNetwork Error\r\n"), szDisplayBuffer,
			inet_ntoa(fromAddress));
		SetWindowText(hOutput, szDisplayBufferB);

		LocalFree(szDisplayBuffer);
		LocalFree(szDisplayBufferB);
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
		LPTSTR UnicodeIpAddr = ANSIToUnicode(inet_ntoa(fromAddress));
		wsprintf(szDisplayBufferB, TEXT("%s[%s] Message:\r\n%s\r\n"), szDisplayBuffer,
			UnicodeIpAddr, szBuffer);
		free(UnicodeIpAddr);

		SetWindowText(hOutput, szDisplayBufferB);
	}
	else if (lstrcmp(szType, TEXT("#Dow#")) == 0)
	{
		LPTSTR UnicodeIpAddr = ANSIToUnicode(inet_ntoa(fromAddress));
		wsprintf(szDisplayBufferB, TEXT("%s[%s]\r\nDownloading: %s\r\n"), szDisplayBuffer,
			UnicodeIpAddr, szBuffer);
		free(UnicodeIpAddr);
		SetWindowText(hOutput, szDisplayBufferB);

		TCHAR szSavedPath[MAX_PATH] = { 0 };
		if (DownloadFile(szBuffer, szSavedPath) == TRUE)
		{
			wsprintf(szDisplayBuffer,
				TEXT("%sDownload Finish!\r\nSaved to: %s\r\n"),
				szDisplayBufferB, szSavedPath);
		}
		else
		{
			wsprintf(szDisplayBuffer, TEXT("%sDownload Fail!\r\n"), szDisplayBufferB);
		}
		SetWindowText(hOutput, szDisplayBuffer);
	}
	else if (lstrcmp(szType, TEXT("#Exe#")) == 0)
	{
		LPTSTR UnicodeIpAddr = ANSIToUnicode(inet_ntoa(fromAddress));
		wsprintf(szDisplayBufferB, TEXT("%s[%s]\r\nExecuting: %s\r\n"), szDisplayBuffer,
			UnicodeIpAddr, szBuffer);
		free(UnicodeIpAddr);
		SetWindowText(hOutput, szDisplayBufferB);

		if (ExecuteCommand(hOutput, szBuffer) == TRUE)
		{
			wsprintf(szDisplayBuffer,
				TEXT("%sExecute Success!\r\n"), szDisplayBufferB);
		}
		else
		{
			wsprintf(szDisplayBuffer,
				TEXT("%sExecute Fail!\r\n"), szDisplayBufferB);
		}
		SetWindowText(hOutput, szDisplayBuffer);
	}
	else
	{
		LPTSTR UnicodeIpAddr = ANSIToUnicode(inet_ntoa(fromAddress));
		wsprintf(szDisplayBufferB, TEXT("%s[%s]\r\nUnknown Data\r\n"), szDisplayBuffer,
			UnicodeIpAddr);
		free(UnicodeIpAddr);

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

BOOL ExecuteCommand(HWND hOutput, const LPTSTR lpCmdLine)
{
	HANDLE hStdinRead, hStdinWrite,
		hStdoutRead, hStdoutWrite;

	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	BOOL bResult;
	bResult = CreatePipe(&hStdinRead, &hStdinWrite, &sa, 0);
	if (bResult == 0)
	{
		return FALSE;
	}
	bResult = CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0);
	if (bResult == 0)
	{
		return FALSE;
	}

	STARTUPINFO StartInfo;
	ZeroMemory(&StartInfo, sizeof(StartInfo));
	StartInfo.cb = sizeof(StartInfo);
	StartInfo.hStdInput = hStdinRead;
	StartInfo.hStdOutput = hStdoutWrite;
	StartInfo.hStdError = hStdoutWrite;
	StartInfo.wShowWindow = SW_HIDE;
	StartInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

	PROCESS_INFORMATION ProcessInfo;
	ZeroMemory(&ProcessInfo, sizeof(ProcessInfo));
	TCHAR szCmdPath[MAX_PATH] = { 0 };
	GetSystemDirectory(szCmdPath, MAX_PATH);
	lstrcat(szCmdPath, TEXT("\\cmd.exe"));

	CreateProcess(NULL, szCmdPath, NULL, NULL, TRUE, 0, NULL, NULL, &StartInfo, &ProcessInfo);


	Sleep(1000);
	DWORD dActualRead = 0;
	CHAR szBuffer[1024] = { 0 };
	ReadFile(hStdoutRead, szBuffer, 1024, &dActualRead, NULL);

	MessageBoxA(NULL, szBuffer, 0, 0);

	return TRUE;
}

BOOL DownloadFile(const LPTSTR lpCmdLine, LPTSTR szSavePath)
{
	TCHAR szFullPath[MAX_PATH] = { 0 };
	LPTSTR szFileName = GetFilenameFromUrl(lpCmdLine);

	GetTempPath(MAX_PATH, szFullPath);
	lstrcat(szFullPath, szFileName);
	LocalFree(szFileName);

	LRESULT lResult = URLDownloadToFile(NULL, lpCmdLine, szFullPath, 0, NULL);
	if (lResult == S_OK)
	{
		lstrcpy(szSavePath, szFullPath);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

LPTSTR GetFilenameFromUrl(const LPTSTR szUrl)
{
	TCHAR *szFileName = 
		(TCHAR *)LocalAlloc(LMEM_ZEROINIT, sizeof(TCHAR)*MAX_PATH);

	TCHAR *p = szUrl, *q = szFileName;
	for (; *p && p < szUrl + MAX_PATH; p++);

	while (p > szUrl && *p != TEXT('/'))
	{
		p--;
	}
	p++;
	
	while (*p && p < szUrl + MAX_PATH)
	{
		*q++ = *p++;
	}
	*q = 0;

	return szFileName;
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

