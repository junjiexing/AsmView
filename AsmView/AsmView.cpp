
#define _WIN32_WINNT 0x400
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include "AsmView.h"
#include "AsmViewInternal.h"
#include "racursor.h"

//
//	Constructor for AsmView class
//
AsmView::AsmView(HWND hwnd)
{
	m_hWnd = hwnd;

	// Font-related data
	m_nNumFonts		= 1;
	m_nHeightAbove	= 0;
	m_nHeightBelow	= 0;
	
	// File-related data
	m_nLineCount   = 0;
	m_nLongestLine = 0;	
	

	// Scrollbar related data
	m_nVScrollPos = 0;
	m_nHScrollPos = 0;
	m_nVScrollMax = 0;
	m_nHScrollMax = 0;

	// Display-related data
	m_nTabWidthChars = 4;
	m_uStyleFlags	 = 0;
	m_nCaretWidth	 = 0;
	m_nLongLineLimit = 80;
	m_nLineInfoCount = 0;

	SystemParametersInfo(SPI_GETCARETWIDTH, 0, &m_nCaretWidth, 0);

	if(m_nCaretWidth == 0)
		m_nCaretWidth = 2;

	// Default display colours
	m_rgbColourList[TXC_FOREGROUND]		= SYSCOL(COLOR_WINDOWTEXT);
	m_rgbColourList[TXC_BACKGROUND]		= SYSCOL(COLOR_WINDOW);
	m_rgbColourList[TXC_HIGHLIGHTTEXT]	= SYSCOL(COLOR_HIGHLIGHTTEXT);
	m_rgbColourList[TXC_HIGHLIGHT]		= SYSCOL(COLOR_HIGHLIGHT);
	m_rgbColourList[TXC_HIGHLIGHTTEXT2]	= SYSCOL(COLOR_INACTIVECAPTIONTEXT);
	m_rgbColourList[TXC_HIGHLIGHT2]		= SYSCOL(COLOR_INACTIVECAPTION);
	m_rgbColourList[TXC_SELMARGIN1]		= SYSCOL(COLOR_3DFACE);
	m_rgbColourList[TXC_SELMARGIN2]		= SYSCOL(COLOR_3DHIGHLIGHT);
	m_rgbColourList[TXC_LINENUMBERTEXT]	= SYSCOL(COLOR_3DSHADOW);
	m_rgbColourList[TXC_LINENUMBER]		= SYSCOL(COLOR_3DFACE);
	m_rgbColourList[TXC_LONGLINETEXT]	= SYSCOL(COLOR_3DSHADOW);
	m_rgbColourList[TXC_LONGLINE]		= SYSCOL(COLOR_3DFACE);
	m_rgbColourList[TXC_CURRENTLINETEXT] = SYSCOL(COLOR_WINDOWTEXT);
	m_rgbColourList[TXC_CURRENTLINE]	 = RGB(230,240,255);


	// Runtime data
	m_nSelectionMode	= SELMODE_NONE;
	m_nScrollTimer		= 0;
	m_fHideCaret		= false;
	m_fTransparent		= true;
	m_hImageList		= 0;
	
	m_nSelectionStart	= 0;
	m_nSelectionEnd		= 0;
	m_nCursorOffset		= 0;
	m_nCurrentLine		= 0;

	m_nLinenoWidth		= 0;
	SetRect(&m_rcBorder, 2, 2, 2, 2);

	m_pTextDoc = new TextDocument();

	m_hMarginCursor = CreateCursor(GetModuleHandle(0), 21, 5, 32, 32, XORMask, ANDMask);
	
	//
	//	The AsmView state must be fully initialized before we
	//	start calling member-functions
	//

	// Set the default font
	OnSetFont((HFONT)GetStockObject(ANSI_FIXED_FONT));

	UpdateMetrics();
	UpdateMarginWidth();
}

//
//	Destructor for AsmView class
//
AsmView::~AsmView()
{
	if(m_pTextDoc)
		delete m_pTextDoc;

	DestroyCursor(m_hMarginCursor);
}

VOID AsmView::UpdateMetrics()
{
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	OnSize(0, rect.right, rect.bottom);
	RefreshWindow();

	RepositionCaret();
}

LONG AsmView::OnSetFocus(HWND hwndOld)
{
	CreateCaret(m_hWnd, (HBITMAP)NULL, m_nCaretWidth, m_nLineHeight);
	RepositionCaret();

	ShowCaret(m_hWnd);
	RefreshWindow();
	return 0;
}

LONG AsmView::OnKillFocus(HWND hwndNew)
{
	// if we are making a selection when we lost focus then
	// stop the selection logic
	if(m_nSelectionMode != SELMODE_NONE)
	{
		OnLButtonUp(0, 0, 0);
	}

	HideCaret(m_hWnd);
	DestroyCaret();
	RefreshWindow();
	return 0;
}

ULONG AsmView::SetStyle(ULONG uMask, ULONG uStyles)
{
	ULONG oldstyle = m_uStyleFlags;

	m_uStyleFlags  = (m_uStyleFlags & ~uMask) | uStyles;

	// update display here
	UpdateMetrics();
	RefreshWindow();

	return oldstyle;
}

ULONG AsmView::SetVar(ULONG nVar, ULONG nValue)
{
	return 0;//oldval;
}

ULONG AsmView::GetVar(ULONG nVar)
{
	return 0;
}

ULONG AsmView::GetStyleMask(ULONG uMask)
{
	return m_uStyleFlags & uMask;
}
	
bool AsmView::CheckStyle(ULONG uMask)
{
	return (m_uStyleFlags & uMask) ? true : false;
}

int AsmView::SetCaretWidth(int nWidth)
{
	int oldwidth = m_nCaretWidth;
	m_nCaretWidth  = nWidth;

	return oldwidth;
}

BOOL AsmView::SetImageList(HIMAGELIST hImgList)
{
	m_hImageList = hImgList;
	return TRUE;
}

LONG AsmView::SetLongLine(int nLength)
{
	int oldlen = m_nLongLineLimit;
	m_nLongLineLimit = nLength;
	return oldlen;
}

int CompareLineInfo(LINEINFO *elem1, LINEINFO *elem2)
{
	if(elem1->nLineNo < elem2->nLineNo)
		return -1;
	if(elem1->nLineNo > elem2->nLineNo)
		return 1;
	else
		return 0;
}

int AsmView::SetLineImage(ULONG nLineNo, ULONG nImageIdx)
{
	LINEINFO *linfo = GetLineInfo(nLineNo);

	// if already a line with an image
	if(linfo)
	{
		linfo->nImageIdx = nImageIdx;
	}
	else
	{
		linfo = &m_LineInfo[m_nLineInfoCount++];
		linfo->nLineNo = nLineNo;
		linfo->nImageIdx = nImageIdx;

		// sort the array
		qsort(
			m_LineInfo, 
			m_nLineInfoCount, 
			sizeof(LINEINFO), 
			(COMPAREPROC)CompareLineInfo
			);

	}
	return 0;
}

LINEINFO* AsmView::GetLineInfo(ULONG nLineNo)
{
	LINEINFO key = { nLineNo, 0 };

	// perform the binary search
	return (LINEINFO *)	bsearch(
							&key, 
							m_LineInfo,
							m_nLineInfoCount, 
							sizeof(LINEINFO), 
							(COMPAREPROC)CompareLineInfo
							);
}


//
//	Public memberfunction 
//
LONG WINAPI AsmView::WndProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Draw contents of AsmView whenever window needs updating
	case WM_PAINT:
		return OnPaint();

	// Set a new font 
	case WM_SETFONT:
		return OnSetFont((HFONT)wParam);

	case WM_SIZE:
		return OnSize(wParam, LOWORD(lParam), HIWORD(lParam));

	case WM_VSCROLL:
		return OnVScroll(LOWORD(wParam), HIWORD(wParam));

	case WM_HSCROLL:
		return OnHScroll(LOWORD(wParam), HIWORD(wParam));

	case WM_MOUSEACTIVATE:
		return OnMouseActivate((HWND)wParam, LOWORD(lParam), HIWORD(lParam));

	case WM_MOUSEWHEEL:
		return OnMouseWheel((short)HIWORD(wParam));

	case WM_SETFOCUS:
		return OnSetFocus((HWND)wParam);

	case WM_KILLFOCUS:
		return OnKillFocus((HWND)wParam);

	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, (short)LOWORD(lParam), (short)HIWORD(lParam));

	case WM_SETCURSOR:
		if(LOWORD(lParam) == HTCLIENT)
			return TRUE;
		else
			break;

	case WM_TIMER:
		return OnTimer(wParam);

	//
	case TXM_OPENFILE:
		return OpenFile((TCHAR *)lParam);

	case TXM_CLEAR:
		return ClearFile();

	case TXM_SETLINESPACING:
		return SetLineSpacing(wParam, lParam);

	case TXM_ADDFONT:
		return AddFont((HFONT)wParam);

	case TXM_SETCOLOR:
		return SetColour(wParam, lParam);

	case TXM_SETSTYLE:
		return SetStyle(wParam, lParam);

	case TXM_SETCARETWIDTH:
		return SetCaretWidth(wParam);

	case TXM_SETIMAGELIST:
		return SetImageList((HIMAGELIST)wParam);

	case TXM_SETLONGLINE:
		return SetLongLine(lParam);

	case TXM_SETLINEIMAGE:
		return SetLineImage(wParam, lParam);

	default:
		break;
	}

	return DefWindowProc(m_hWnd, msg, wParam, lParam);
}

//
//	Win32 AsmView window procedure stub
//
LRESULT WINAPI AsmViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AsmView *ptv = (AsmView *)GetWindowLongPtr(hwnd, 0);

	switch(msg)
	{
	// First message received by any window - make a new AsmView object
	// and store pointer to it in our extra-window-bytes
	case WM_NCCREATE:

		if((ptv = new AsmView(hwnd)) == 0)
			return FALSE;

		SetWindowLongPtr(hwnd, 0, (LONG)ptv);
		return TRUE;

	// Last message received by any window - delete the AsmView object
	case WM_NCDESTROY:
		delete ptv;
		SetWindowLongPtr(hwnd, 0, 0);
		return 0;

	// Pass everything to the AsmView window procedure
	default:
		if(ptv)
			return ptv->WndProc(msg, wParam, lParam);
		else
			return 0;
	}
}

//
//	Register the AsmView window class
//
BOOL InitAsmView()
{
	WNDCLASSEX	wcx;

	//Window class for the main application parent window
	wcx.cbSize			= sizeof(wcx);
	wcx.style			= 0;
	wcx.lpfnWndProc		= AsmViewWndProc;
	wcx.cbClsExtra		= 0;
	wcx.cbWndExtra		= sizeof(AsmView *);
	wcx.hInstance		= GetModuleHandle(0);
	wcx.hIcon			= 0;
	wcx.hCursor			= LoadCursor (NULL, IDC_IBEAM);
	wcx.hbrBackground	= (HBRUSH)0;		//NO FLICKERING FOR US!!
	wcx.lpszMenuName	= 0;
	wcx.lpszClassName	= AsmView_CLASS;	
	wcx.hIconSm			= 0;

	return RegisterClassEx(&wcx) ? TRUE : FALSE;
}

//
//	Create a AsmView control!
//
HWND CreateAsmView(HWND hwndParent)
{
	return CreateWindowEx(WS_EX_CLIENTEDGE, 
		AsmView_CLASS, _T(""), 
		WS_VSCROLL |WS_HSCROLL | WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, 
		hwndParent, 
		0, 
		GetModuleHandle(0), 
		0);
}

