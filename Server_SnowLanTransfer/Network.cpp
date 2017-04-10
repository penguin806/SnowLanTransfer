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
		return INVALID_SOCKET;
	}

	return Sock;
}

BOOL SendData(INT Sock, ULONG Address, LPWSTR szData, ULONG iDataSize)
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

VOID CleanNetwork(INT Sock)
{
	if (Sock != INVALID_SOCKET || Sock != 0)
	{
		closesocket(Sock);
	}

	WSACleanup();
}