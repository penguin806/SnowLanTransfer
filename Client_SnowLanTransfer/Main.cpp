// Snow 2017-04-10
#include "Header.h"
#include "resource.h"

#define WM_TRAYICON WM_USER + 1

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT iCmdShow)
{
	INT Sock;

	Sock = InitNetwork();
	if (Sock == INVALID_SOCKET || Sock == 0)
	{
		MessageBox(NULL, TEXT("Windows Socket Init Fail!"), TEXT("Fatal Error"),
			MB_OK | MB_ICONERROR);
		return 0;
	}

	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SNOW_MAINWND), NULL, MainWndProc,
		(LPARAM)Sock);

	return 0;
}

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hNetThread;
	static THREAD_DATA DataToPass;
	static NOTIFYICONDATA TrayData;
	static BOOL bHide;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		AddTrayIcon(hMainWnd, &TrayData, sizeof(NOTIFYICONDATA));
		DataToPass.hOutput = GetDlgItem(hMainWnd, IDC_OUTPUT);
		DataToPass.Sock = (INT)lParam;
		hNetThread = CreateThread(NULL, 0, NetThreadProc, &DataToPass, 0, NULL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			TerminateThread(hNetThread, 0);
			CloseHandle(hNetThread);
			Sleep(500);
			CleanNetwork(DataToPass.Sock);

			Shell_NotifyIcon(NIM_DELETE, &TrayData);
			EndDialog(hMainWnd, 0);
			return TRUE;
		default:
			break;
		}
		break;
	case WM_SYSCOMMAND:
		if (wParam == SC_MINIMIZE)
		{
			ShowWindow(hMainWnd, SW_HIDE);
			bHide = TRUE;
			return TRUE;
		}
		break;
	case WM_TRAYICON:
		if (lParam == WM_LBUTTONDOWN)
		{
			if (bHide == FALSE)
			{
				ShowWindow(hMainWnd, SW_HIDE);
				bHide = TRUE;
			}
			else
			{
				ShowWindow(hMainWnd, SW_NORMAL);
				bHide = FALSE;
			}
		}
		break;
	default:
		break;
	}

	return 0;
}

VOID AddTrayIcon(HWND hMainWnd, NOTIFYICONDATA *Data, UINT uSize)
{
	ZeroMemory(Data, uSize);
	Data->uID = 0;
	Data->cbSize = uSize;
	Data->hWnd = hMainWnd;
	Data->hIcon = LoadIcon(NULL, IDI_APPLICATION);
	Data->uCallbackMessage = WM_TRAYICON;
	lstrcpy(Data->szTip, TEXT("Client - Snow Lan Transfer v1.0"));
	Data->uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	Shell_NotifyIcon(NIM_ADD, Data);
}