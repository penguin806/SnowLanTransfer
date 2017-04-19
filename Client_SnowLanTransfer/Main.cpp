// Snow 2017-04-10
#include "Header.h"
#include "resource.h"

#define WM_TRAYICON WM_USER + 1

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT iCmdShow)
{
	SNOWDATA data;
	ZeroMemory(&data, sizeof(data));

	BOOL bResult = InitNetwork(&data);
	if (bResult == FALSE)
	{
		MessageBox(NULL, TEXT("Windows Socket Init Fail!"), TEXT("Fatal Error"),
			MB_OK | MB_ICONERROR);
		return 0;
	}

	DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_SNOW_MAINWND), NULL, MainWndProc,
		(LPARAM)&data);

	return 0;
}

INT_PTR CALLBACK MainWndProc(HWND hMainWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hNetThread;
	static SNOWDATA DataToPass;
	static NOTIFYICONDATA TrayData;
	static BOOL bHide;

	switch (uMsg)
	{
	case WM_INITDIALOG:
		CopyMemory(&DataToPass, (const void *)lParam, sizeof(SNOWDATA));
		DataToPass.hOutput = GetDlgItem(hMainWnd, IDC_OUTPUT);
		AddTrayIcon(hMainWnd, &TrayData, sizeof(NOTIFYICONDATA));
		
		hNetThread = CreateThread(NULL, 0, NetThreadProc, &DataToPass, 0, NULL);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			TerminateThread(hNetThread, 0);
			CloseHandle(hNetThread);
			Sleep(500);
			CleanNetwork(DataToPass.Sock, DataToPass.msgSock);

			Shell_NotifyIcon(NIM_DELETE, &TrayData);
			EndDialog(hMainWnd, 0);
			return TRUE;
		case IDC_CHECK_AUTOSTART:
		{
			if (HIWORD(wParam) == BN_CLICKED)
			{
				if (SendDlgItemMessage(hMainWnd, IDC_CHECK_AUTOSTART, BM_GETCHECK, 0, 0) == BST_CHECKED)
				{
					StartWithWindows(TRUE);
				}
				else
				{
					StartWithWindows(FALSE);
				}
			}
			break;
		}
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

VOID StartWithWindows(BOOL bSwitch)
{
	HKEY hRegKey = NULL;
	LONG lResult = RegOpenKeyEx(HKEY_CURRENT_USER,
		TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0,
		KEY_WRITE, &hRegKey);

	if (lResult != ERROR_SUCCESS || hRegKey == NULL)
	{
		return;
	}

	if (bSwitch == TRUE)
	{
		TCHAR szPath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szPath, MAX_PATH - 1);
		RegSetValueEx(hRegKey, TEXT("Client-SnowLan"), 0, REG_SZ,
			(BYTE *)szPath, sizeof(szPath));
	}
	else
	{
		RegDeleteValue(hRegKey, TEXT("Client-SnowLan"));
	}

	RegCloseKey(hRegKey);
}