// Snow 2017-04-10
#include "Header.h"
#include "resource.h"

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

	switch (uMsg)
	{
	case WM_INITDIALOG:
		DataToPass.hOutput = GetDlgItem(hMainWnd, IDC_OUTPUT);
		DataToPass.Sock = (INT)lParam;
		hNetThread = CreateThread(NULL, 0, NetThreadProc, &DataToPass, 0, NULL);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDCANCEL:
			TerminateThread(hNetThread, 0);
			CloseHandle(hNetThread);
			CleanNetwork(DataToPass.Sock);
			EndDialog(hMainWnd, 0);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	return 0;
}