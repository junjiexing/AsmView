
#include <windows.h>
#include <tchar.h>
#include "AsmView.h"
#include "AsmViewInternal.h"

//
//	
//
LONG AsmView::OpenFile(TCHAR *szFileName)
{
	ClearFile();

	if(m_pTextDoc->init(szFileName))
	{
		m_nLineCount   = m_pTextDoc->linecount();
		m_nLongestLine = m_pTextDoc->longestline(4);

		m_nVScrollPos  = 0;
		m_nHScrollPos  = 0;

		m_nSelectionStart	= 0;
		m_nSelectionEnd		= 0;
		m_nCursorOffset		= 0;

		UpdateMetrics();
		UpdateMarginWidth();
		return TRUE;
	}

	return FALSE;
}

//
//
//
LONG AsmView::ClearFile()
{
	if(m_pTextDoc)
		m_pTextDoc->clear();

	m_nLineCount   = m_pTextDoc->linecount();
	m_nLongestLine = m_pTextDoc->longestline(4);

	m_nVScrollPos  = 0;
	m_nHScrollPos  = 0;

	UpdateMetrics();

	return TRUE;
}
