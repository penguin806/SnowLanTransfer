// Snow 2017-04-10
#include "Header.h"
#include "resource.h"

#define BUFFER_LEN 512
#define OPTION_NUM 3

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, INT iCmdShow)
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
	static INT Sock;
	const LPTSTR szComboText[OPTION_NUM] = {
		TEXT("Send Message"),
		TEXT("Download File"),
		TEXT("Execute Command")
	};

	switch (uMsg)
	{
	case WM_INITDIALOG:
		Sock = (INT)lParam;

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
			CleanNetwork(Sock);
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

			BOOL bResult = SendData(Sock, IpAddr, szDataToSend, sizeof(szDataToSend));
			if (bResult == FALSE)
			{
				MessageBox(hMainWnd, TEXT("FAIL"), TEXT("Error"), MB_OK | MB_ICONERROR);
			}
			else
			{
				MessageBox(hMainWnd, TEXT("SUCCEED"), TEXT("Finish"), MB_OK | MB_ICONINFORMATION);
			}
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