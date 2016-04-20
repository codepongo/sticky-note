#include "stdafx.h"
#include "Mnemosyne-notes.h"
#include <Windows.h>
#include <shellapi.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <wchar.h>
#include <atlconv.h>
#define ID_TRAY WM_USER+20
#define IDM_CHANGE WM_USER+51
#define WM_USER_SHELLICON WM_USER+1
#define MAX_NOTE_SIZE 10000

wchar_t note[MAX_NOTE_SIZE];
NOTIFYICONDATA nid;
HMENU hPopMenu;
HWND hWnd;
LPCWSTR	szTitle = L"Mnemosyne-notes";				// The title bar text
LPCWSTR	szWindowClass = L"Mnemosyne-notes";			// the main window class name
LPCWSTR	icon_path_name = L"icon.ico";

HFONT font = CreateFont(20, 8, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_ROMAN, L"Tahoma");

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Edit(HWND, UINT, WPARAM, LPARAM);
void SaveNote();
void LoadNote();

//MAIN
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{

	MyRegisterClass(hInstance);

	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MNEMOSYNENOTES));
	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MNEMOSYNENOTES));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	DWORD styleEx = WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW;
	DWORD style = WS_POPUP | WS_VISIBLE;
	hWnd = CreateWindowEx(styleEx, szWindowClass, szTitle, style, GetSystemMetrics(SM_CXSCREEN) - 400, 10, 400, 800, NULL, NULL, hInstance, NULL);
	SetLayeredWindowAttributes(hWnd, 0x00ffffff, 255, LWA_COLORKEY | LWA_ALPHA | SW_SHOWMINIMIZED);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SW_SHOWMINIMIZED);
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	DWORD icon_flags = LR_LOADFROMFILE | LR_DEFAULTSIZE;
	HICON hicon = (HICON)LoadImage(hInstance, icon_path_name, IMAGE_ICON, 0, 0, icon_flags);

	LPCWSTR sTip = L"Mnemosyne-notes";
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = (HWND)hWnd;
	nid.uID = ID_TRAY;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_USER_SHELLICON;
	nid.hIcon = hicon;
	lstrcpy(nid.szTip, sTip);
	Shell_NotifyIcon(NIM_ADD, &nid);
	return TRUE;
}

POINT lpClickPoint;
UINT uFlag;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		LoadNote();
		break;
	case WM_USER_SHELLICON:
		switch (LOWORD(lParam)) {
		case WM_RBUTTONDOWN:
			uFlag = MF_BYPOSITION | MF_STRING;
			GetCursorPos(&lpClickPoint);
			hPopMenu = CreatePopupMenu();
			InsertMenu(hPopMenu, 0xFFFFFFFF, uFlag, IDM_CHANGE, _T("Change note"));
			InsertMenu(hPopMenu, 0xFFFFFFFF, uFlag, IDM_EXIT, _T("Exit"));
			SetForegroundWindow(hWnd);
			TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);

			break;

		case WM_LBUTTONDOWN:
			SetForegroundWindow(hWnd);
			break;
		}break;

	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		switch (wmId)
		{
		case IDM_CHANGE:
			DialogBox((HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE), MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Edit);
			break;

		case IDM_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;

		case IDM_ABOUT:
			DialogBox((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}break;

	case WM_PAINT:
	{
		RECT rect;
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		SetTextColor(hdc, RGB(254, 254, 254));
		GetClientRect(hWnd, &rect);
		rect.left = 0;
		rect.top = 0;
		SelectObject(hdc, font);
		DrawText(hdc, note, -1, &rect, DT_NOCLIP | DT_CENTER);
		DeleteDC(hdc);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Edit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_EDIT1, note);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case IDOK:
			UpdateWindow(hWnd);
			RECT rect;
			GetClientRect(hWnd, &rect);
			rect.left = 0;
			rect.top = 0;
			GetDlgItemText(hDlg, IDC_EDIT1, note, MAX_NOTE_SIZE);
			SaveNote();
			InvalidateRect(hWnd, &rect, true);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;

		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
			break;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void SaveNote() {

	std::wstring wsNote(note);
	std::string strNote(wsNote.begin(), wsNote.end());
	WideCharToMultiByte (CP_ACP, 0, &wsNote[0], (int)wsNote.size(), &strNote[0], MAX_NOTE_SIZE, NULL, NULL);
	std::ofstream ofs("Note.txt");
	ofs << strNote;
	ofs.close();
}

void LoadNote() {
	std::string strNote, line;
	strNote = "";
	std::ifstream ifs("Note.txt");
	if (ifs.is_open())
	{
		while (getline(ifs, line))
		{
			strNote += line;
			strNote += '/n';
		}
		ifs.close();
	}
	MultiByteToWideChar(CP_ACP, 0, strNote.c_str(), -1, note, MAX_NOTE_SIZE);
}