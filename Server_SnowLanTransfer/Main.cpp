// Snow 2017-04-10
#include "Header.h"
#include "resource.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, INT iCmdShow)
{
	INT Sock, recvSock;

	Sock = InitNetwork();
	if (Sock == INVALID_SOCKET || Sock == 0)
	{
		MessageBox(NULL, TEXT("Windows Socket Init Fail!"), TEXT("Fatal Error"),
			MB_OK | MB_ICONERROR);
		return 0;
	}

	recvSock = InitListenClient();
	if (Sock == INVALID_SOCKET || Sock == 0)
	{
		MessageBox(NULL, TEXT("Cannot Listen Client!"), TEXT("Error"), MB_OK | MB_ICONERROR);
	}

	SNOWDATA data;
	ZeroMemory(&data, sizeof(data));
	data.Sock = Sock;
	data.recvSock = recvSock;

	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SNOW_MAINWND), NULL, MainWndProc,
		(LPARAM)&data);

	return 0;
}

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static SNOWDATA Data;
	const LPTSTR szComboText[OPTION_NUM] = {
		TEXT("Send Message"),
		TEXT("Download File"),
		TEXT("Execute Command")
	};
	static HANDLE hListenClientThread;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		CopyMemory(&Data, (const void *)lParam, sizeof(SNOWDATA));
		Data.hOutput = GetDlgItem(hMainWnd, IDC_OUTPUT);
		Data.hFile = CreateLogFile();
		if (Data.hFile == INVALID_HANDLE_VALUE || Data.hFile == NULL)
		{
			MessageBox(hMainWnd, TEXT("Unable to Create Log File"), TEXT("Error"),
				MB_OK | MB_ICONERROR);
		}

		hListenClientThread = CreateThread(NULL, 0,
			ClientMessageProc, (LPVOID)&Data, 0, NULL);

		if (Data.Sock == 0 || Data.recvSock == 0)
		{
			MessageBox(NULL, TEXT("Unknown Error"), TEXT("ERROR"),
				MB_OK | MB_ICONERROR);
			EndDialog(hMainWnd, 0);
			return 0;
		}

		for (int i = 0; i < OPTION_NUM; i++)
		{
			SendDlgItemMessage(hMainWnd, IDC_COMBO1, CB_INSERTSTRING,
				-1, (LPARAM)szComboText[i]);
		}
		SendDlgItemMessage(hMainWnd, IDC_COMBO1, CB_SELECTSTRING,
			0, (LPARAM)szComboText[0]);
		SetDlgItemText(hMainWnd, IDC_IPADDRESS, TEXT("192.168.1.255"));
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			if (hListenClientThread)
			{
				TerminateThread(hListenClientThread, 0);
				CloseHandle(hListenClientThread);
			}

			if (Data.hFile)
			{
				CloseHandle(Data.hFile);
			}
			CleanNetwork(Data);
			EndDialog(hMainWnd, 0);
			break;
		case IDC_SEND:
		{
			CHAR szAsciiBuffer[BUFFER_LEN];
			ZeroMemory(szAsciiBuffer, sizeof(szAsciiBuffer));

			GetDlgItemTextA(hMainWnd, IDC_IPADDRESS, szAsciiBuffer, sizeof(szAsciiBuffer));
			ULONG IpAddr = inet_addr(szAsciiBuffer);
			if (IpAddr == INADDR_NONE || IpAddr == INADDR_ANY)
			{
				MessageBox(hMainWnd, TEXT("Ip Address Format Error"), TEXT("Error"),
					MB_OK | MB_ICONINFORMATION);
				break;
			}

			TCHAR szBuffer[BUFFER_LEN], szDataToSend[BUFFER_LEN];
			UINT iCurSel;

			ZeroMemory(szBuffer, sizeof(szBuffer));
			ZeroMemory(szDataToSend, sizeof(szDataToSend));
			
			iCurSel = SendDlgItemMessage(hMainWnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);
			if (iCurSel >= OPTION_NUM)
			{
				break;
			}

			GetDlgItemText(hMainWnd, IDC_INPUT, szBuffer, BUFFER_LEN);
			if (lstrcmp(szBuffer, TEXT("")) == 0)
			{
				MessageBox(hMainWnd, TEXT("Input Area Empty!"), TEXT("Error"),
					MB_OK | MB_ICONINFORMATION);
				break;
			}

			wsprintf(szDataToSend, TEXT("#%.3s#%s"), szComboText[iCurSel], szBuffer);

			BOOL bResult = SendData(Data.Sock, IpAddr, szDataToSend, sizeof(szDataToSend));
			if (bResult == FALSE)
			{
				TCHAR szIpAddress[BUFFER_LEN];
				GetDlgItemText(hMainWnd, IDC_IPADDRESS, szIpAddress, BUFFER_LEN);
				wsprintf(szBuffer, TEXT("SEND TO %s FAIL:\r\n%s\r\n"), szIpAddress,
					szDataToSend);
				MessageBox(hMainWnd, TEXT("FAIL"), TEXT("Error"), MB_OK | MB_ICONERROR);
			}
			else
			{
				TCHAR szIpAddress[BUFFER_LEN];
				GetDlgItemText(hMainWnd, IDC_IPADDRESS, szIpAddress, BUFFER_LEN);
				wsprintf(szBuffer, TEXT("SEND TO %s SUCCESS:\r\n%s\r\n"), szIpAddress,
					szDataToSend);
				MessageBox(hMainWnd, TEXT("SUCCEED"), TEXT("Finish"), MB_OK | MB_ICONINFORMATION);
			}
			WriteLogToFile(Data.hFile, szBuffer);
			break;
		}
		default:
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}