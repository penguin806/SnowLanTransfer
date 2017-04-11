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
		if (iResult > 0)
		{
			MessageBox(NULL, szBuffer, 0, 0);
		}
	}

	return 0;
}


VOID CleanNetwork(INT Sock)
{
	if (Sock != INVALID_SOCKET || Sock != 0)
	{
		closesocket(Sock);
	}

	WSACleanup();
}