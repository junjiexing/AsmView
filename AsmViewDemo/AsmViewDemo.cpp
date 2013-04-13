// AsmViewDemo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "AsmViewDemo.h"
#include "..\AsmView\AsmView.h"
#include <commctrl.h>
#include <CommDlg.h>


#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
HWND g_hwndAsmView = NULL;

// 此代码模块中包含的函数的前向声明:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: 在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_ASMVIEWDEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	InitAsmView();

	// 执行应用程序初始化:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASMVIEWDEMO));

	// 主消息循环:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
//  注释:
//
//    仅当希望
//    此代码与添加到 Windows 95 中的“RegisterClassEx”
//    函数之前的 Win32 系统兼容时，才需要此函数及其用法。调用此函数十分重要，
//    这样应用程序就可以获得关联的
//    “格式正确的”小图标。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASMVIEWDEMO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_ASMVIEWDEMO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: 处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDM_OPEN:
			{
				char *szFilter		= _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0\0");
				char szFileName[MAX_PATH] = {0};
				char szTitleName[MAX_PATH] = {0};

				OPENFILENAME ofn	= { sizeof(ofn) };

				ofn.hwndOwner		= hWnd;
				ofn.hInstance		= GetModuleHandle(0);
				ofn.lpstrFilter		= szFilter;
				ofn.lpstrFile		= szFileName;
				ofn.lpstrFileTitle	= szTitleName;

				ofn.nFilterIndex	= 1;
				ofn.nMaxFile		= _MAX_PATH;
				ofn.nMaxFileTitle	= _MAX_FNAME + _MAX_EXT; 

				// flags to control appearance of open-file dialog
				ofn.Flags			=	OFN_EXPLORER			| 
					OFN_ENABLESIZING		|
					OFN_ALLOWMULTISELECT	| 
					OFN_FILEMUSTEXIST;

				if (GetOpenFileName(&ofn))
				{
					if(AsmView_OpenFile(g_hwndAsmView, szFileName))
					{
						SetWindowText(hWnd, szTitleName);
						return TRUE;
					}
					else
					{
						MessageBox(hWnd, _T("Error opening file"), NULL, MB_ICONEXCLAMATION);
						return FALSE;
					}
				}
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此添加任意绘图代码...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		{
			g_hwndAsmView = CreateAsmView(hWnd);

			HIMAGELIST hImgList = ImageList_LoadImage(
				hInst, 
				MAKEINTRESOURCE(IDB_BITMAP1), 
				16, 0, 
				RGB(255,0,255),
				IMAGE_BITMAP,
				LR_LOADTRANSPARENT|LR_CREATEDIBSECTION
				);

			AsmView_SetImageList(g_hwndAsmView, hImgList);
			AsmView_SetLineImage(g_hwndAsmView, 5,  2);

			AsmView_SetLineSpacing(g_hwndAsmView, 5, 5);

			AsmView_SetStyleBool(g_hwndAsmView, TXS_SELMARGIN,	TRUE);
			AsmView_SetStyleBool(g_hwndAsmView, TXS_LINENUMBERS,	FALSE);
			AsmView_SetStyleBool(g_hwndAsmView, TXS_LONGLINES,	FALSE);
			AsmView_SetStyleBool(g_hwndAsmView, TXS_HIGHLIGHTCURLINE,	TRUE);
			AsmView_SetCaretWidth(g_hwndAsmView, 2);

			HDC hdc      = GetDC(0);
			int nLogSize = 0-MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			ReleaseDC(0, hdc);
			HFONT hFont = CreateFont(nLogSize, 
				0, 0, 0, 
				0,
				0,0,0,0,0,0,
				0,
				0,
				"新宋体");
			SendMessage(g_hwndAsmView, WM_SETFONT, (WPARAM)hFont, 0);

			//AsmView_OpenFile(g_hwndAsmView, "D:\\1.txt");
			break;
		}
	case WM_SIZE:
		{
			short width  = (short)LOWORD(lParam);
			short height = (short)HIWORD(lParam);

			MoveWindow(g_hwndAsmView, 0, 0, width, height, TRUE);
			break;
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
