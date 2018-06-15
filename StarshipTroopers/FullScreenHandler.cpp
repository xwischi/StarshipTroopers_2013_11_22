////////////////////////////////////////////////////////////////
// MSDN Magazine -- December 2002
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
// Compiles with VC 6.0 or VS.NET on Windows XP. Tab size=3.
//
#include "StdAfx.h"
#include "FullScreenHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// redfine this to nop if you don't want tracing
#define FSTRACE TRACE

// Global object handles full-screen mode
CFullScreenHandler FullScreenHandler;

CFullScreenHandler::CFullScreenHandler()
{
	m_rcRestore.SetRectEmpty();
}

CFullScreenHandler::~CFullScreenHandler()
{
}

//////////////////
// Resize frame so view's client area fills the entire screen. Use
// GetSystemMetrics to get the screen size -- the rest is pixel
// arithmetic.
//
void CFullScreenHandler::Maximize(CFrameWnd* pFrame, CWnd* pView)
{
	// get view rectangle
	if (pView)
	{
		CRect rcv;
		pView->GetWindowRect(&rcv);

		// get frame rectangle
		pFrame->GetWindowRect(m_rcRestore); // save for restore
		const CRect& rcf = m_rcRestore;				// frame rect

		FSTRACE("Frame=(%d,%d) x (%d,%d)\n", rcf.left, rcf.top, rcf.Width(), rcf.Height());
		FSTRACE("View =(%d,%d) x (%d,%d)\n", rcv.left, rcv.top, rcv.Width(), rcv.Height());

		// now compute new rect
		CRect rc(0,0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

		FSTRACE("Scrn =(%d,%d) x (%d,%d)\n", rc.left, rc.top, rc.Width(), rc.Height());
//		rc.top   += -50;

		rc.left  += rcf.left  - rcv.left;
		rc.top   += rcf.top   - rcv.top;
		rc.right += rcf.right - rcv.right;
		rc.bottom+= rcf.bottom- rcv.bottom;

		FSTRACE("New  =(%d,%d) x (%d,%d)\n", rc.left, rc.top, rc.Width(), rc.Height());

		// move frame!
		pFrame->SetWindowPos(NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER);
	}
}

void CFullScreenHandler::Restore(CFrameWnd* pFrame)
{
	CRect &rc = m_rcRestore;
	//rc.bottom += 49;
	pFrame->SetWindowPos(NULL, rc.left, rc.top,	rc.Width(), rc.Height(), SWP_NOZORDER);
//	pFrame->SetWindowPos(NULL, rc.left, rc.top,	rc.Width(), rc.Height(), SWP_NOZORDER);
	m_rcRestore.SetRectEmpty();
}


CSize CFullScreenHandler::GetMaxSize()
{
	CRect rc(0, 0, GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
	rc.InflateRect(10,50);
	return rc.Size();
}
